import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';
import * as speculosButtons from '../utils/speculosButtons';
import {cryptoUtils, Signature} from "@hiveio/dhive";
import {PublicKey} from "@hiveio/dhive/lib/crypto";

const prepareExpectedSignature = (msg: string, expectedSignature: string) => ({msg, expectedSignature});

describe('Sign message', async () => {
    [
        // short message
        prepareExpectedSignature('Test', '1f52721df9ed62bf7eb064547a3fa2443dc850e82eda7baa434c9fa1243f608155208ab694cc8e71295e593a2d4b5a33563eb4544b8d1c626f04e65951c49e2464'),
        // max length message
        prepareExpectedSignature('x'.repeat(512), '2065458416d886ecc537eebb10dba459bd537fd3d651a877398d9dba38fda63ecb2412beca7e967de5f15ebc6a17b5f2d528f35ba4b7489b64a6a26dadc30ff91c'),
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

    it('should generate signature compatible with Hive Keychain', async function () {
        const transport = await Transport.open({ apduPort: 40000, buttonPort: 5000, automationPort: 5000 });
        try {
            const hive = new Hive(transport);

            const message = 'This is a message to sign';

            const publicKey = await hive.getPublicKey(`48'/13'/0'/0'/0'`);
            const signingMsgPromise = hive.signMessage(message, `48'/13'/0'/0'/0'`);

            // accept message
            await speculosButtons.pressLeft();
            await speculosButtons.pressLeft();
            await speculosButtons.pressBoth();

            const signature = await signingMsgPromise;
            const key = PublicKey.fromString(publicKey);
            const isSignedByKey = key.verify(cryptoUtils.sha256(message), Signature.fromString(signature));
            expect(isSignedByKey).to.be.true;
        } finally {
            await transport.close();
        }
    });
})