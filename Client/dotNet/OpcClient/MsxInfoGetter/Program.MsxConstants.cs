namespace Konamiman.Opc.MsxInfoGetter
{
    partial class Program
    {
        readonly string[] MsxVersions = { "MSX1", "MSX2", "MSX2+", "MSX Turbo-R" };
        readonly string[] VdpTypes = { "TMS9918/TMS9929", "V9938", "V9948", "V9958" };


        //BIOS and DOS entry points

        const ushort RDSLT = 0x000C;  //Return in A value read from address HL, slot A
        const ushort CALSLT = 0x001C; //Return in A value read from address IX, slot IYh
        const ushort DOS = 0xF37D;
        const ushort H_PHYD = 0x0FFA7;
        const ushort EXTBIO = 0xFFCA;


        //DOS function calls

        const byte _DOSVER = 0x6F;


        //Addresses with interesting info

        const ushort VDPREADPORT = 0x0006;
        const ushort VDPWRITEPORT = 0x0007;

        const ushort MSXVER = 0x002D;

        const ushort EXPTBL = 0xFCC1;
    }
}
