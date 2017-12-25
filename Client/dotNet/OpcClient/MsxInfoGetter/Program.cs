using Konamiman.Opc.ClientLibrary;
using Konamiman.Z80dotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Konamiman.Opc.MsxInfoGetter
{
    class Program
    {
        static void Main(string[] args)
        {
            var transport = new TcpTransport("localhost", 12345);
            var opcClient = new OpcClient(transport);
            transport.Connect();

            var inRegs = new Z80Registers();
            inRegs.C = 0x6F;
            //var outRegs = opcClient.Execute(0x200, inRegs);
            //var result = opcClient.Ping(7);
            //var mem = opcClient.ReadFromMemory(0x1234, 15);
            //mem = opcClient.ReadFromMemory(0x1234, 256);
            opcClient.WriteToMemory(0x1000, new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 });
            //var mem = opcClient.ReadFromMemory(0x9000, 20);
            //var mem = opcClient.ReadFromPort(0x98, 7, false);
            //mem = opcClient.ReadFromPort(0x98, 7, true);
            //mem = opcClient.ReadFromPort(0x98, 300, false);
            //mem = opcClient.ReadFromPort(0x98, 300, true);

            opcClient.WriteToPort(0x98, new byte[] { 1, 2, 3, 4, 5 }, false);
            opcClient.WriteToPort(0x98, new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }, false);
            opcClient.WriteToPort(0x98, new byte[] { 1, 2, 3, 4, 5 }, true);
            //opcClient.WriteToPort(0x98, new byte[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }, true);


        }
    }
}
