using Konamiman.Z80dotNet;
using NUnit.Framework;
using System;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    [TestFixture(Category = "Read Memory")]
    public class ReadMemoryTests : TestsBase
    {
        [Test]
        [TestCase(7, true)]
        [TestCase(7, false)]
        [TestCase(0x1234, true)]
        [TestCase(0x1234, false)]
        public void Sends_proper_command_bytes(int size, bool lockAddress)
        {
            ushort address = 0xABCD;

            var expectedToSend = size <= 7 ?
                new byte[] {
                    (byte)(0x20 | size | (lockAddress ? (1 << 3) : 0)),
                    address.GetLowByte(), address.GetHighByte() }
                :
                new byte[] {
                    (byte)(0x20 | (lockAddress ? (1 << 3) : 0)),
                    address.GetLowByte(), address.GetHighByte(),
                    size.ToUShort().GetLowByte(), size.ToUShort().GetHighByte() };

            var toReceive = Enumerable.Repeat<byte>(0, size + 1).ToArray();
            CreateSut(toReceive);

            sut.ReadFromMemory(address, size, lockAddress);

            CollectionAssert.AreEqual(expectedToSend, transport.SentBytes);
        }

        [Test]
        [TestCase(7)]
        [TestCase(0x1234)]
        public void Returns_received_data_properly(int size)
        {
            var toReceive = Enumerable.Range(0, size + 1).Select(x => (byte)x).ToArray();
            CreateSut(toReceive);

            var received = sut.ReadFromMemory(RandomAddress, size);

            CollectionAssert.AreEqual(toReceive.Skip(1).ToArray(), received);
        }

        [Test]
        public void Throws_on_server_error()
        {
            AssertThrowsOpcError(() => sut.ReadFromMemory(RandomAddress, 1));
        }

        [Test]
        public void Throws_on_null_buffer()
        {
            CreateSut(0);
            Assert.Throws<ArgumentNullException>(() => sut.ReadFromMemory(RandomAddress, null, 0, 1));
        }

        [Test]
        [TestCase(-1, 1)]
        [TestCase(1, -1)]
        [TestCase(11, 1)]
        [TestCase(9, 2)]
        public void Throws_on_bad_size_or_index(int index, int size)
        {
            CreateSut(0);
            var buffer = new byte[10];
            Assert.Throws<ArgumentOutOfRangeException>(() => sut.ReadFromMemory(RandomAddress, buffer, index, size));
        }
    }
}
