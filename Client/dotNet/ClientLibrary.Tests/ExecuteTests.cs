using Konamiman.Z80dotNet;
using NUnit.Framework;
using System.Linq;

namespace Konamiman.Opc.ClientLibrary.Tests
{
    [TestFixture(Category = "Execute")]
    public class ExecuteTests : TestsBase
    {
        [Test]
        [TestCase(Z80RegistersGroup.Af, 2)]
        [TestCase(Z80RegistersGroup.Main, 8)]
        [TestCase(Z80RegistersGroup.MainIndex, 12)]
        [TestCase(Z80RegistersGroup.MainIndexAlternate, 20)]
        public void Sends_proper_input_registers_information(Z80RegistersGroup inputRegistersGroup, int inputRegistersSize)
        {
            const int address = 0xABCD;
            const short randomAF = 0;

            var inputRegisters = new Z80Registers
            {
                AF = 0x1122,
                BC = 0x3344,
                DE = 0x5566,
                HL = 0x7788,
                IX = 0x99AA.ToShort(),
                IY = 0xBBCC.ToShort()
            };
            inputRegisters.Alternate.AF = 0x1234;
            inputRegisters.Alternate.BC = 0x5678;
            inputRegisters.Alternate.DE = 0x9ABC.ToShort();
            inputRegisters.Alternate.HL = 0xDEF0.ToShort();

            var commandByte = (byte)(0x10 | ((int)Z80RegistersGroup.Af << 2) | ((int)inputRegistersGroup << 0));
            var expectedToBeSent = new byte[]
            {
                commandByte,
                address.ToShort().GetLowByte(), address.ToShort().GetHighByte(),
                0x22, 0x11,
                0x44, 0x33, 0x66, 0x55, 0x88, 0x77,
                0xAA, 0x99, 0xCC, 0xBB,
                0x34, 0x12, 0x78, 0x56, 0xBC, 0x9A, 0xF0, 0xDE
            }
            .Take(inputRegistersSize + 3).ToArray();

            CreateSut(0, randomAF.GetLowByte(), randomAF.GetHighByte());

            var outputRegisters = sut.Execute(address, inputRegisters, inputRegistersGroup, Z80RegistersGroup.Af);

            CollectionAssert.AreEqual(expectedToBeSent, transport.SentBytes);
        }

        [Test]
        [TestCase(Z80RegistersGroup.Af, 2)]
        [TestCase(Z80RegistersGroup.Main, 8)]
        [TestCase(Z80RegistersGroup.MainIndex, 12)]
        [TestCase(Z80RegistersGroup.MainIndexAlternate, 20)]
        public void Returns_proper_output_registers_information(Z80RegistersGroup outputRegistersGroup, int outputRegistersSize)
        {
            const Z80RegistersGroup randomInputRegistersGroup = Z80RegistersGroup.Af;
            var randomInputRegisters = new Z80Registers();

            var toBeReceived = new byte[]
            {
                0x00,
                0x22, 0x11,
                0x44, 0x33, 0x66, 0x55, 0x88, 0x77,
                0xAA, 0x99, 0xCC, 0xBB,
                0x34, 0x12, 0x78, 0x56, 0xBC, 0x9A, 0xF0, 0xDE
            }
            .Take(outputRegistersSize + 1).ToArray();

            CreateSut(toBeReceived);

            var outputRegisters = sut.Execute(RandomAddress, randomInputRegisters, randomInputRegistersGroup, outputRegistersGroup);

            Assert.AreEqual(0x1122, outputRegisters.AF);

            if(outputRegistersGroup >= Z80RegistersGroup.Main)
            {
                Assert.AreEqual(0x3344, outputRegisters.BC);
                Assert.AreEqual(0x5566, outputRegisters.DE);
                Assert.AreEqual(0x7788, outputRegisters.HL);
            }

            if (outputRegistersGroup >= Z80RegistersGroup.MainIndex)
            {
                Assert.AreEqual(0x99AA.ToShort(), outputRegisters.IX);
                Assert.AreEqual(0xBBCC.ToShort(), outputRegisters.IY);
            }

            if (outputRegistersGroup >= Z80RegistersGroup.MainIndexAlternate)
            {
                Assert.AreEqual(0x1234.ToShort(), outputRegisters.Alternate.AF);
                Assert.AreEqual(0x5678.ToShort(), outputRegisters.Alternate.BC);
                Assert.AreEqual(0x9ABC.ToShort(), outputRegisters.Alternate.DE);
                Assert.AreEqual(0xDEF0.ToShort(), outputRegisters.Alternate.HL);
            }
        }

        [Test]
        public void Throws_on_server_error()
        {
            AssertThrowsOpcError(() => sut.Execute(0, new Z80Registers()));
        }
    }
}
