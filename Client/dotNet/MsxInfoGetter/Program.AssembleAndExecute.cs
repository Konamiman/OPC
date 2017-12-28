using Konamiman.Opc.ClientLibrary;
using Konamiman.Z80dotNet;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Konamiman.Opc.MsxInfoGetter
{
    partial class Program
    {
        private Z80Registers AssembleAndExecute(string program, ushort address = ushort.MaxValue)
        {
            if (address == ushort.MaxValue) address = executionAddress;

            program = $" org 0{address:X}h\r\n{program}";

            var tempPath = Path.GetTempPath();
            var asmFile = Path.Combine(tempPath, "MsxInfoGetter_temp.asm");
            File.WriteAllText(asmFile, program);

            var startInfo = new ProcessStartInfo("sjasm.exe", $"-s {asmFile}")
            {
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                UseShellExecute = false
            };
            var proc = new Process();
            proc.StartInfo = startInfo;
            proc.Start();
            proc.WaitForExit();

            if (proc.ExitCode != 0)
            {
                var output = proc.StandardOutput.ReadToEnd();
                output = RemoveFirstLine(output);
                output = Regex.Replace(output, @"temp\.asm\(([0-9]+)\) :", "Line $1 :");
                throw new Exception($"Assembly process failed:\r\n\r\n{output}");
            }

            var bytes = File.ReadAllBytes(Path.Combine(tempPath, "MsxInfoGetter_temp.out"));

            client.WriteToMemory(address, bytes);
            return client.Execute(address, regs, Z80RegistersGroup.Af);
        }

        private static string RemoveFirstLine(string output)
        {
            return string.Join(
                Environment.NewLine,
                output.Split(new[] { Environment.NewLine }, StringSplitOptions.None).Skip(1).ToArray());
        }
    }
}
