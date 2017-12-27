using Konamiman.Opc.ClientLibrary;
using Konamiman.Z80dotNet;
using System;
using System.Globalization;
using System.IO;
using static System.Console;

namespace Konamiman.Opc.MsxInfoGetter
{
    partial class Program
    {
        OpcClient client;
        byte biosSlot;
        Z80Registers regs = new Z80Registers();
        byte vdpReadPort;
        byte vdpWritePort;
        ushort executionAddress;

        static void Main(string[] args)
        {
            try
            {
#if DEBUG
                new Program().Run(new[] { "localhost", "12345", "9000" });
                ReadKey();
#else
                new Program().Run(args);
#endif
            }
            catch(Exception ex)
            {
                WriteLine("*** " + ex.Message);
#if DEBUG
                ReadKey();
#endif
            }
            finally
            {
                foreach (var file in Directory.EnumerateFiles(Path.GetTempPath(), "MsxInfoGetter_temp.*"))
                    File.Delete(file);
            }
        }

        void Run(string[] args)
        {
            Write(
@"MSX information getter 1.0
By Konamiman, 1/2018
");

            if(args.Length < 2)
            {
                WriteLine(
@"
Gets system information from a MSX computer running a OPC server on a TCP port.
Usage: msxinfo <OPC server address> <OPC server port> [<code execution address in hex>]");
                return;
            }

            if(!ushort.TryParse(args[1], out ushort port))
            {
                WriteLine("*** Invalid port number");
                return;
            }

            if(args.Length == 2)
            {
                executionAddress = 0x8000;
            }
            else if (!ushort.TryParse(args[2], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out executionAddress))
            {
                WriteLine("*** Invalid execution address");
                return;
            }

            var transport = new TcpTransport(args[0], port);
            client = new OpcClient(transport);
            transport.Connect();

            Ping();

            biosSlot = client.ReadFromMemory(EXPTBL, 1)[0];

            WriteLine();
            byte msxVersion = PrintMsxVersion();
            WriteLine();
            if (msxVersion > 0)
            {
                PrintVdpType();
                PrintVdpPorts();
                WriteLine();
            }
            PrintDosVersion();
            PrintMappedMemoryAmount();
        }
        
        private byte PrintMsxVersion()
        {
            var msxVersion = Rdslt(biosSlot, MSXVER);
            var printableMsxVersion =
                msxVersion >= MsxVersions.Length ?
                $"Unknown ({msxVersion})" :
                MsxVersions[msxVersion];
            WriteLine($"MSX system version: {printableMsxVersion}");
            return msxVersion;
        }

        private void PrintVdpType()
        {
            var code =
$@"
 di

 ;Set 1 (S# to read) to R#15, then read VDP port #1
 ld a,1
 out ({vdpWritePort + 1}),a
 ld a,8Fh
 out ({vdpWritePort + 1}),a
 in a,({vdpReadPort + 1})

 ;R#15 needs to be set to 0 again, otherwise the MSX will hang
 ld b,a
 xor a
 out ({vdpWritePort + 1}),a
 ld a,8Fh
 out ({vdpWritePort + 1}),a
 ld a,b

 ei
 ret
";

            var outRegs = AssembleAndExecute(code);
            var vdpType = (outRegs.A & 62) / 2 + 1;
            var printableVdpType =
                vdpType >= VdpTypes.Length ?
                $"Unknown ({vdpType})" :
                VdpTypes[vdpType];

            WriteLine($"VDP type: {printableVdpType}");
        }

        private void PrintVdpPorts()
        {
            vdpReadPort = Rdslt(biosSlot, VDPREADPORT);
            WriteLine($"VDP read port: 0x{vdpReadPort:X}");
            vdpWritePort = Rdslt(biosSlot, VDPWRITEPORT);
            WriteLine($"VDP write port: 0x{vdpWritePort:X}");
        }


        private void PrintDosVersion()
        {
            var phydHook = client.ReadFromMemory(H_PHYD, 1)[0];
            if (phydHook == 0xC9)
                return;  //no disk system present -> no DOS

            regs.B = 0x5A;
            regs.C = _DOSVER;
            regs.HL = 0x1234;
            regs.DE = 0xABCD.ToShort();
            regs.IX = 0;

            string osTypeAndVersion;

            var outRegs = client.Execute(DOS, regs, Z80RegistersGroup.MainIndex, Z80RegistersGroup.MainIndex);

            if (outRegs.A != 0)
                osTypeAndVersion = "Unknown";
            else if (outRegs.A == 1 && regs.B == 1)
                osTypeAndVersion = $"Nextor {outRegs.IXL}.{outRegs.IYH}.{outRegs.IYL} in MSX-DOS 1 mode";
            else if (outRegs.B < 2)
                osTypeAndVersion = "MSX-DOS 1";
            else if(outRegs.IX == 0)
                osTypeAndVersion = $"MSX-DOS {outRegs.B}.{outRegs.C:X2}";
            else if(outRegs.IXH == 1)
                osTypeAndVersion = $"Nextor {outRegs.IXL}.{outRegs.IYH}.{outRegs.IYL}";
            else
                osTypeAndVersion = "Unknown";

            WriteLine($"OS: {osTypeAndVersion}");
        }

        private void PrintMappedMemoryAmount()
        {
            var code =
$@"
 ld a,(0F313h)
 or a
 ld hl,0
 ret z ;TODO: Calculate for DOS 1

 xor a
 ld de,0401h
 call {EXTBIO}
 push hl
 pop ix
 ld hl,0

;IX=Pointer to first entry on a table of 8-byte entries.
;   For each entry:
;   +0 = slot number (0 = end of table)
;   +1 = # of RAM segments on that slot
;   +2 = # of free segments (TODO: return that as well)

LOOP:
 ld a,(ix)
 or a
 ret z

 ld c,(ix+1)
 ld b,0
 add hl,bc
 ld bc,8
 add ix,bc
 jr LOOP
";

            var outRegs = AssembleAndExecute(code);
            if (outRegs.HL > 0)
                WriteLine($"Mapped RAM: {outRegs.HL * 16}K");
        }

        private void Ping()
        {
            var param = new Random().Next(1, 15);
            var result = client.Ping(param);
            if ((result[0] & 0x0F) != param)
                throw new Exception("Unexpected response to PING (not an OPC server?)");
        }

        byte Rdslt(byte slot, ushort address)
        {
            regs.HL = address.ToShort();
            regs.A = biosSlot;
            var outRegs = client.Execute(RDSLT, regs);
            return outRegs.A;
        }
    }
}
