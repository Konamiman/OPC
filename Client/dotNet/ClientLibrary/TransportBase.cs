using System;

namespace Konamiman.Opc.ClientLibrary
{
    public abstract class TransportBase : ITransport
    {
        public abstract int Receive(byte[] buffer, int index, int size);
        public abstract int Send(byte[] buffer, int index, int size);
        protected void ValidateParameters(byte[] buffer, int index, int size)
        {
            if (buffer == null)
                throw new ArgumentNullException($"{nameof(buffer)} can't be null");

            if (index < 0)
                throw new ArgumentOutOfRangeException($"{nameof(index)} can't be negative");

            if (size < 0)
                throw new ArgumentOutOfRangeException($"{nameof(size)} can't be negative");

            if (index >= buffer.Length)
                throw new ArgumentOutOfRangeException($"{nameof(index)} can't be greater than the size of {nameof(buffer)}");

            if (index + size > buffer.Length)
                throw new ArgumentOutOfRangeException($"{nameof(index)}+{nameof(size)} can't be greater than the size of {nameof(buffer)}");
        }
    }
}
