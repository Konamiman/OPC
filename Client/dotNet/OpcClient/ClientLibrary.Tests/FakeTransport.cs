using System;
using System.Collections.Generic;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    public class FakeTransport : TransportBase
    {
        private readonly byte[] receiveBuffer;
        private readonly List<byte> sendBuffer = new List<byte>();
        private int receiveBufferIndex = 0;
        private int remainingInBuffer;

        public FakeTransport(byte[] receiveBuffer)
        {
            this.receiveBuffer = receiveBuffer;
            remainingInBuffer = receiveBuffer.Length;
        }

        public override int Receive(byte[] buffer, int index, int size)
        {
            ValidateParameters(buffer, index, size);

            if (remainingInBuffer == 0)
                return 0;

            var actualSize = Math.Min(size, remainingInBuffer);
            Array.Copy(receiveBuffer, receiveBufferIndex, buffer, index, actualSize);
            receiveBufferIndex += actualSize;
            remainingInBuffer -= actualSize;

            return actualSize;
        }

        public override int Send(byte[] buffer, int index, int size)
        {
            ValidateParameters(buffer, index, size);

            sendBuffer.AddRange(buffer.Skip(index).Take(size).ToArray());
            return size;
        }

        public byte[] SentBytes => sendBuffer.ToArray();
    }
}
