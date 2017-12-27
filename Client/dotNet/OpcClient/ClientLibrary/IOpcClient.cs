using Konamiman.Z80dotNet;

namespace Konamiman.Opc.ClientLibrary
{
    /// <summary>
    /// Represents an OPC client.
    /// </summary>
    public interface IOpcClient
    {
        /// <summary>
        /// Sends a "Ping" command to the OPC server.
        /// </summary>
        /// <param name="commandParameter">Value to be sent in the low nibble of the command parameter.</param>
        /// <returns>Data received from the server (not including the initial 0 byte).</returns>
        /// <exception cref="System.ArgumentOutOfRangeException">commandParameter is less than zero or greater than 15.</exception>
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        byte[] Ping(int commandParameter);

        /// <summary>
        /// Sends a "Execute code" command to the OPC server.
        /// </summary>
        /// <param name="address">Address of the code to execute.</param>
        /// <param name="inputRegistersGroup">Z80 registers to load before executing the command.</param>
        /// <param name="inputRegisters">Value of the Z80 registers to load before executing the command.</param>
        /// <param name="outputRegistersGroup">Z80 registers to return after executing the command.</param>
        /// <param name="outputRegisters">Value of the Z80 registers after executing the command.</param>
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        void Execute(
            ushort address,
            Z80RegistersGroup inputRegistersGroup,
            Z80Registers inputRegisters,
            Z80RegistersGroup outputRegistersGroup,
            Z80Registers outputRegisters);

        /// <summary>
        /// Sends a "Read from memory" command to the OPC server.
        /// </summary>
        /// <param name="address">Memory address to start reading.</param>
        /// <param name="buffer">Storage location for the received data.</param>
        /// <param name="index">Position in the data buffer to store the received data.</param>
        /// <param name="size">Number of bytes to receive.</param>
        /// <param name="lockAddress">If set, all the data will be read from the same memory address.</param>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        /// <remarks>The client will wait until all the requested data has been received before returning.
        /// If not all data can be received it will throw <see cref="TransportException"/>
        /// and close the connection (if applicable).
        /// </remarks>
        void ReadFromMemory(
            ushort address,
            byte[] buffer,
            int index,
            int size,
            bool lockAddress = false);

        /// <summary>
        /// Sends a "Write to memory" command to the OPC server.
        /// </summary>
        /// <param name="address">Memory address to start writing.</param>
        /// <param name="buffer">Buffer with the data to be sent.</param>
        /// <param name="index">Position in the data buffer at which to begin sending data.</param>
        /// <param name="size">Number of bytes to send.</param>
        /// <param name="lockAddress">If set, all the data will be written to the same memory address.</param>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>        
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        /// <remarks>The client will wait until all the requested data has been sent before returning.
        /// If not all data can be sent it will throw <see cref="TransportException"/>
        /// and close the connection (if applicable).
        /// </remarks>
        void WriteToMemory(
            ushort address,
            byte[] buffer,
            int index,
            int size,
            bool lockAddress = false);

        /// <summary>
        /// Sends a "Read from port" command to the OPC server.
        /// </summary>
        /// <param name="port">Port (or first port) to read data from.</param>
        /// <param name="buffer">Storage location for the received data.</param>
        /// <param name="index">Position in the data buffer to store the received data.</param>
        /// <param name="size">Number of bytes to receive.</param>
        /// <param name="autoIncrement">True to increment the port number after reading each byte of data.
        /// False to read all the data from the port specified initially.</param>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        /// <remarks>The client will wait until all the requested data has been received before returning.
        /// If not all data can be received it will throw <see cref="TransportException"/>
        /// and close the connection (if applicable).
        /// </remarks>
        void ReadFromPort(
            byte port,
            byte[] buffer,
            int index,
            int size,
            bool autoIncrement);

        /// <summary>
        /// Sends a "Write to port" command to the OPC server.
        /// </summary>
        /// <param name="port">Port (or first port) to write data to.</param>
        /// <param name="buffer">Buffer with the data to be sent.</param>
        /// <param name="index">Position in the data buffer at which to begin sending data.</param>
        /// <param name="size">Number of bytes to send.</param>
        /// <param name="autoIncrement">True to increment the port number after writing each byte of data.
        /// False to write all the data to the port specified initially.</param>
        /// <exception cref="System.ArgumentNullException">Buffer is null.</exception>
        /// <exception cref="System.ArgumentOutOfRangeException">Offset is less than 0.-or- offset is greater than the length of buffer.-or- size
        /// is less than 0.-or- size is greater than the length of buffer minus the value
        /// of the offset parameter.</exception>        
        /// <exception cref="TransportException">Error at the transport level.</exception>
        /// <exception cref="OpcException">The server returned an error message.</exception>
        /// <remarks>The client will wait until all the requested data has been sent before returning.
        /// If not all data can be sent it will throw <see cref="TransportException"/>
        /// and close the connection (if applicable).
        /// </remarks>
        void WriteToPort(
            byte port,
            byte[] buffer,
            int index,
            int size,
            bool autoIncrement);
    }
}
