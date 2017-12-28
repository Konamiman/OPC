using Konamiman.Z80dotNet;
using System;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary
{
    public static class OpcClientExtensions
    {
        public static Z80Registers Execute(
            this IOpcClient client,
            ushort address,
            Z80Registers inputRegisters,
            Z80RegistersGroup inputRegistersGroup = Z80RegistersGroup.Main,
            Z80RegistersGroup outputRegistersGroup = Z80RegistersGroup.Main)
        {

            Z80Registers outputRegisters = new Z80Registers();
            client.Execute(
                address,
                inputRegistersGroup, 
                inputRegisters,
                outputRegistersGroup,
                outputRegisters);

            return outputRegisters;
        }

        public static byte[] ReadFromMemory(this IOpcClient client, ushort address, int size, bool lockAddress = false)
        {
            var buffer = new byte[size];
            client.ReadFromMemory(address, buffer, 0, size, lockAddress);
            return buffer;
        }

        public static void WriteToMemory(this IOpcClient client, ushort address, byte[] data, bool lockAddress = false)
        {
            if (data == null)
                throw new ArgumentNullException($"{nameof(data)} can't be null");

            client.WriteToMemory(address, data, 0, data.Length, lockAddress);
        }

        public static byte[] ReadFromPort(this IOpcClient client, byte port, int size, bool autoIncrement)
        {
            var buffer = new byte[size];
            client.ReadFromPort(port, buffer, 0, size, autoIncrement);
            return buffer;
        }

        public static void WriteToPort(this IOpcClient client, byte port, byte[] data, bool autoIncrement)
        {
            if (data == null)
                throw new ArgumentNullException($"{nameof(data)} can't be null");

            client.WriteToPort(port, data, 0, data.Length, autoIncrement);
        }
    }
}
