using System;
using System.Collections.Generic;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    public class FakeTransport : ITransport
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

        public int Receive(byte[] buffer, int index, int size)
        {
            if (remainingInBuffer == 0)
                return 0;

            var actualSize = Math.Min(size, remainingInBuffer);
            Array.Copy(receiveBuffer, receiveBufferIndex, buffer, index, actualSize);
            receiveBufferIndex += actualSize;
            remainingInBuffer -= actualSize;

            return actualSize;
        }

        public int Send(byte[] buffer, int index, int size)
        {
            sendBuffer.AddRange(buffer.Skip(index).Take(size).ToArray());
            return size;
        }

        public byte[] SentBytes => receiveBuffer.ToArray();
    }
}
