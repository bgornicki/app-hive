import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';
import * as speculosButtons from '../utils/speculosButtons';

const prepareExpectedSignature = (msg: string, expectedSignature: string) => ({msg, expectedSignature});

describe('Sign message', async () => {
    [
        prepareExpectedSignature('Test', '1f52721df9ed62bf7eb064547a3fa2443dc850e82eda7baa434c9fa1243f608155208ab694cc8e71295e593a2d4b5a33563eb4544b8d1c626f04e65951c49e2464'),
        prepareExpectedSignature('{"msg": "This is a longer string imitating JSON"}', '1f28bcb4ca91fdc92c407a63d297911e92814226ba79ce7803c7aaf9a1a055a2bc2e9591da1518be7f55c88317b7032a05e28bb47904c2fcc130ac9d32bfe797f5'),
    ].forEach(input => {
        it(`should properly sign message`, async function () {
            const transport = await Transport.open({ apduPort: 40000, buttonPort: 5000, automationPort: 5000 });
            try {
                const hive = new Hive(transport);

                const signingMsgPromise = hive.signMessage(input.msg, `48'/13'/0'/0'/0'`);

                // accept message
                await speculosButtons.pressLeft();
                await speculosButtons.pressLeft();
                await speculosButtons.pressBoth();

                const signature = await signingMsgPromise;
                expect(signature).to.be.equal(input.expectedSignature);
            } finally {
                await transport.close();
            }
        }).timeout(10000)
    })


})