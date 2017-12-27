using Konamiman.Z80dotNet;
using NUnit.Framework;
using System;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    [TestFixture(Category = "Write Memory")]
    public class WriteMemoryTests : TestsBase
    {
        [Test]
        [TestCase(7, true)]
        [TestCase(7, false)]
        [TestCase(0x1234, true)]
        [TestCase(0x1234, false)]
        public void Sends_proper_command_bytes(int size, bool lockAddress)
        {
            ushort address = 0xABCD;

            var bytesToWrite = Enumerable.Range(1, size).Select(x => (byte)x).ToArray();

            var expectedToSend = (size <= 7 ?
                new byte[] {
                    (byte)(0x30 | size | (lockAddress ? (1 << 3) : 0)),
                    address.GetLowByte(), address.GetHighByte() }
                :
                new byte[] {
                    (byte)(0x30 | (lockAddress ? (1 << 3) : 0)),
                    address.GetLowByte(), address.GetHighByte(),
                    size.ToUShort().GetLowByte(), size.ToUShort().GetHighByte() })

                .Concat(bytesToWrite).ToArray();
            
            CreateSut(0);

            sut.WriteToMemory(address, bytesToWrite, lockAddress);

            CollectionAssert.AreEqual(expectedToSend, transport.SentBytes);
        }

        [Test]
        public void Throws_on_server_error()
        {
            AssertThrowsOpcError(() => sut.WriteToMemory(RandomAddress, new byte[] {1}));
        }

        [Test]
        public void Throws_on_null_buffer()
        {
            CreateSut(0);
            Assert.Throws<ArgumentNullException>(() => sut.WriteToMemory(RandomAddress, null));

            CreateSut(0);
            Assert.Throws<ArgumentNullException>(() => sut.WriteToMemory(RandomAddress, null, 0, 1));
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
            Assert.Throws<ArgumentOutOfRangeException>(() => sut.WriteToMemory(RandomAddress, buffer, index, size));
        }
    }
}
