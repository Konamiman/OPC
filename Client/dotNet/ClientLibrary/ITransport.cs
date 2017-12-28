using System.Collections.Generic;

namespace Konamiman.Opc.ClientLibrary
{
    /// <summary>
    /// Represents a reliable, sequential, in-order delivery stream of data.
    /// </summary>
    public interface ITransport
    {
        /// <summary>
        /// Receives data from the stream into the specified buffer.
        /// </summary>
        /// <param name="buffer">Storage location for the received data.</param>
        /// <param name="index">Position in the data buffer to store the received data.</param>
        /// <param name="size">Number of bytes to receive.</param>
        /// <returns>Number of bytes received.</returns>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>
        /// <exception cref="Konamiman.Opc.ClientLibrary.TransportException">An error prevented the data from being read (e.g. the connection is not available).</exception>
        int Receive(byte[] buffer, int index, int size);

        /// <summary>
        /// Sends data from the specified buffer to the stream.
        /// </summary>
        /// <param name="buffer">Buffer with the data to be sent.</param>
        /// <param name="index">Position in the data buffer at which to begin sending data.</param>
        /// <param name="size">Number of bytes to send.</param>
        /// <returns>Number of bytes sent.</returns>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>
        /// <exception cref="Konamiman.Opc.ClientLibrary.TransportException">An error prevented the data from being read (e.g. the connection is not available).</exception>
        int Send(byte[] buffer, int index, int size);
    }
}
