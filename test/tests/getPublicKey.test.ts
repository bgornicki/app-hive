import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';
import assert from 'assert';

const prepareExpectedPublicKey = (path: string, expectedKey: string) => ({
    path, expectedKey
})

describe('Get public key', async () => {

    [
        prepareExpectedPublicKey(`48'/13'/0'/0'/0'`, 'STM5m57x4BXEePAzVNrjUqYeh9C2a7eez1Ya2wPo7ngWLQUdEXjKn'),
        prepareExpectedPublicKey(`48'/13'/0'/0'/1'`, 'STM7Yp7fUt268SXCP4u9MS2RPfiGGqyHLQqUGXmCaVqBYX8D6s1ma'),
        prepareExpectedPublicKey(`48'/13'/0'/1'/0'`, 'STM7ZEBoDotbYpnyNHdARYMDBMNnLWpV7fiiGa6pvHbXhfRo9ZrDf'),
        prepareExpectedPublicKey(`48'/13'/0'/1'/1'`, 'STM8DHkcTHodhiNxc9UhvoaTs92A7aWQKKetKvxMRzaA6Kj8XgmRU'),
        prepareExpectedPublicKey(`48'/13'/1'/0'/0'`, 'STM6eNfReAkEHwqAq5fFrPU8e1RdYAW9YqHDKJ1SC4H3M3w7Ne8GJ'),
        prepareExpectedPublicKey(`48'/13'/1231'/0'/0'`, 'STM8MUUUNdXFWtkukCJSYH6tAdao7xSMrgcQSNYnpoFcNgHvoZweG'),
        prepareExpectedPublicKey(`48'/13'/1231'/4312'/0'`, 'STM5MfGQodnuoMVzz2TDH3TTpDRezBwz8NySNLejBYR2mw88p4F5v'),
        prepareExpectedPublicKey(`48'/13'/1231'/4312'/32'`, 'STM78VD38qfLcdpmfPqrS6ZQfvADRqDQvJJ6gEQ9qPfSk8d98YX2M'),
    ].forEach(input => {
        it(`should get public key for path ${input.path}`, async () => {
            const transport = await Transport.open({ apduPort: 40000 });
            try {
                const hive = new Hive(transport);
                const key = await hive.getPublicKey(input.path, false);
                expect(key).to.be.equal(input.expectedKey);
            } finally {
                await transport.close();
            }
        })
    })

    it('should reject path not compiliant with SLIP-0048 (path path len > 5)', async () => {
        const transport = await Transport.open({ apduPort: 40000 });
        const hive = new Hive(transport);
        try {
            await hive.getPublicKey(`48'/13'/0'/0'/0'/0'`, false);
            assert(false);
        } catch (error: any) {
            assert.equal(error.statusCode, 0xB001); // SW_WRONG_BIP32_PATH
        }
        finally {
            await transport.close();
        }
    })

    it('should reject path not assigned to Hive network (SLIP-0048)', async () => {
        const transport = await Transport.open({ apduPort: 40000 });
        const hive = new Hive(transport);
        try {
            await hive.getPublicKey(`48'/12'/0'/0'/0'/0'`, false);
            assert(false);
        } catch (error: any) {
            assert.equal(error.statusCode, 0xB001); // SW_WRONG_BIP32_PATH
        }
        finally {
            await transport.close();
        }
    })

    it('should reject path not compiliant with SLIP-0048', async () => {
        const transport = await Transport.open({ apduPort: 40000 });
        const hive = new Hive(transport);
        try {
            await hive.getPublicKey(`47'/12'/0'/0'/0'/0'`, false);
            assert(false);
        } catch (error: any) {
            assert.equal(error.statusCode, 0xB001); // SW_WRONG_BIP32_PATH
        }
        finally {
            await transport.close();
        }
    })
})