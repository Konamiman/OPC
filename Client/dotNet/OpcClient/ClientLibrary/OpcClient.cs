using System;
using System.Linq;
using System.Text;
using Konamiman.Z80dotNet;

namespace Konamiman.Opc.ClientLibrary
{
    public class OpcClient : IOpcClient
    {
        private readonly ITransport transport;
        private readonly byte[] dataBuffer = new byte[255];
        private static int[] registersLengthsForExecute = {
            1 * 2,
            4 * 2,
            6 * 2,
            10 * 2 };

        public OpcClient(ITransport transport)
        {
            this.transport = transport;
        }

        public byte[] Ping(int commandParameter)
        {
            if (commandParameter < 0 && commandParameter > 15)
                throw new ArgumentOutOfRangeException($"{nameof(commandParameter)} must be a number between 0 and 15");

            Send(commandParameter & 0x0F);
            ReceiveStartOfResponse();

            ReceiveToDataBuffer(1);
            var extraDataLength = dataBuffer[0] >> 4;
            if (extraDataLength > 0)
                ReceiveToDataBuffer(extraDataLength, index: 1);

            return dataBuffer.Take(extraDataLength + 1).ToArray();
        }

        public void Execute(
            ushort address,
            Z80RegistersGroup inputRegistersGroup,
            Z80Registers inputRegisters,
            Z80RegistersGroup outputRegistersGroup,
            Z80Registers outputRegisters)
        {
            dataBuffer[0] = (byte)(0x10 | (int)inputRegistersGroup | ((int)outputRegistersGroup << 2));

            dataBuffer[1] = address.GetLowByte();
            dataBuffer[2] = address.GetHighByte();

            dataBuffer[3] = inputRegisters.F;
            dataBuffer[4] = inputRegisters.A;

            if (inputRegistersGroup >= Z80RegistersGroup.Main)
            {
                dataBuffer[5] = inputRegisters.C;
                dataBuffer[6] = inputRegisters.B;
                dataBuffer[7] = inputRegisters.E;
                dataBuffer[8] = inputRegisters.D;
                dataBuffer[9] = inputRegisters.L;
                dataBuffer[10] = inputRegisters.H;
            }

            if (inputRegistersGroup >= Z80RegistersGroup.MainIndex)
            {
                dataBuffer[11] = inputRegisters.IXL;
                dataBuffer[12] = inputRegisters.IXH;
                dataBuffer[13] = inputRegisters.IYL;
                dataBuffer[14] = inputRegisters.IYH;
            }

            if (inputRegistersGroup >= Z80RegistersGroup.MainIndexAlternate)
            {
                dataBuffer[15] = inputRegisters.Alternate.F;
                dataBuffer[16] = inputRegisters.Alternate.A;
                dataBuffer[17] = inputRegisters.Alternate.C;
                dataBuffer[18] = inputRegisters.Alternate.B;
                dataBuffer[19] = inputRegisters.Alternate.E;
                dataBuffer[20] = inputRegisters.Alternate.D;
                dataBuffer[21] = inputRegisters.Alternate.L;
                dataBuffer[22] = inputRegisters.Alternate.H;
            }

            var sizeToSend = registersLengthsForExecute[(int)inputRegistersGroup] + 3;
            Send(dataBuffer, 0, sizeToSend);
            ReceiveStartOfResponse();

            var sizeToReceive = registersLengthsForExecute[(int)outputRegistersGroup];
            ReceiveToDataBuffer(sizeToReceive);

            outputRegisters.F = dataBuffer[0];
            outputRegisters.A = dataBuffer[1];

            if (outputRegistersGroup >= Z80RegistersGroup.Main)
            {
                outputRegisters.C = dataBuffer[2];
                outputRegisters.B = dataBuffer[3];
                outputRegisters.E = dataBuffer[4];
                outputRegisters.D = dataBuffer[5];
                outputRegisters.L = dataBuffer[6];
                outputRegisters.H = dataBuffer[7];
            }

            if (outputRegistersGroup >= Z80RegistersGroup.MainIndex)
            {
                outputRegisters.IXL = dataBuffer[8];
                outputRegisters.IXH = dataBuffer[9];
                outputRegisters.IYL = dataBuffer[10];
                outputRegisters.IYH = dataBuffer[11];
            }

            if (outputRegistersGroup >= Z80RegistersGroup.MainIndexAlternate)
            {
                outputRegisters.Alternate.F = dataBuffer[12];
                outputRegisters.Alternate.A = dataBuffer[13];
                outputRegisters.Alternate.C = dataBuffer[14];
                outputRegisters.Alternate.B = dataBuffer[15];
                outputRegisters.Alternate.E = dataBuffer[16];
                outputRegisters.Alternate.D = dataBuffer[17];
                outputRegisters.Alternate.L = dataBuffer[18];
                outputRegisters.Alternate.H = dataBuffer[19];
            }
        }

        public void ReadFromMemory(ushort address, byte[] buffer, int index, int size, bool lockAddress = false)
        {
            if (size == 0) return;

            var commandByte = (byte)(0x20 | (lockAddress ? (1 << 3) : 0));
            if (size <= 7)
            {
                Send((byte)(commandByte | size), 
                    address.GetLowByte(), address.GetHighByte());
            }
            else
            {
                var usize = size.ToUShort();
                Send(commandByte,
                    address.GetLowByte(), address.GetHighByte(),
                    usize.GetLowByte(), usize.GetHighByte());
            }

            ReceiveStartOfResponse();

            Receive(size, buffer, index);
        }

        public void ReadFromPort(byte port, byte[] buffer, int index, int size, bool autoIncrement)
        {
            if (size == 0) return;

            var commandByte = (byte)(0x40 | (autoIncrement ? (1 << 3) : 0));
            if (size <= 7)
            {
                Send((byte)(commandByte | size), port);
            }
            else
            {
                var usize = size.ToUShort();
                Send(commandByte, port, usize.GetLowByte(), usize.GetHighByte());
            }

            ReceiveStartOfResponse();

            Receive(size, buffer, index);
        }

        public void WriteToMemory(ushort address, byte[] buffer, int index, int size, bool lockAddress = false)
        {
            if (size == 0) return;

            var commandByte = (byte)(0x30 | (lockAddress ? (1 << 3) : 0));
            if (size <= 7)
            {
                Send((byte)(commandByte | size),
                    address.GetLowByte(), address.GetHighByte());
            }
            else
            {
                var usize = size.ToUShort();
                Send(commandByte,
                    address.GetLowByte(), address.GetHighByte(),
                    usize.GetLowByte(), usize.GetHighByte());
            }

            Send(buffer, index, size);
            ReceiveStartOfResponse();
        }

        public void WriteToPort(byte port, byte[] buffer, int index, int size, bool autoIncrement)
        {
            if (size == 0) return;

            var commandByte = (byte)(0x50 | (autoIncrement ? (1 << 3) : 0));
            if (size <= 7)
            {
                Send((byte)(commandByte | size), port);
            }
            else
            {
                var usize = size.ToUShort();
                Send(commandByte, port, usize.GetLowByte(), usize.GetHighByte());
            }

            Send(buffer, index, size);
            ReceiveStartOfResponse();
        }

        private void Send(params int[] bytes)
        {
            Send(bytes.Select(x => (byte)x).ToArray(), 0, bytes.Length);
        }

        private void Send(byte[] bytes, int index, int size)
        {
            var remaining = size;
            while (remaining > 0)
            {
                var partialSent = transport.Send(bytes, index, remaining);
                index += partialSent;
                remaining -= partialSent;
            }
        }

        private void ReceiveStartOfResponse()
        {
            ReceiveToDataBuffer(1);
            var errorMessageLength = dataBuffer[0];
            if (errorMessageLength == 0)
                return;

            ReceiveToDataBuffer(errorMessageLength);
            var errorMessage = Encoding.ASCII.GetString(dataBuffer, 0, errorMessageLength);
            throw new OpcException(errorMessage);
        }

        private void ReceiveToDataBuffer(int size, int index = 0)
        {
            Receive(size, dataBuffer, index);
        }

        private void Receive(int size, byte[] buffer, int index)
        {
            var remaining = size;
            while (remaining > 0)
            {
                var partialReceived = transport.Receive(buffer, index, remaining);
                index += partialReceived;
                remaining -= partialReceived;
            }
        }
    }
}
