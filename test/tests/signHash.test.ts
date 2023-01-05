import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';
import * as speculosButtons from '../utils/speculosButtons';
import { assert } from 'console';
const {TransportStatusError} = require('@ledgerhq/errors');
const prepareExpectedSignature = (hash: string, expectedSignature: string) => ({
    hash, expectedSignature
})

describe('Sign hash', async () => {

    it('should reject if hash signing is disabled', async () => {

        const transport = await Transport.open({ apduPort: 40000, buttonPort: 5000, automationPort: 5000 });
        try {
            const hive = new Hive(transport);
            await hive.signHash('B2BF27F105D0E0E12F8BC913C8E124B2138E711AFAEAA7E85F186C2D8387F446', `48'/13'/0'/0'/0'`);
            assert(false); // should not reach this point
        } catch(error) {
            const {message, name, statusCode} = error as typeof TransportStatusError;
            const expectedError = new TransportStatusError(45062);

            expect(message).to.be.equal(expectedError.message);
            expect(name).to.be.equal(expectedError.name);
            expect(statusCode).to.be.equal(expectedError.statusCode);
        } finally {
            await speculosButtons.pressRight();
            await speculosButtons.pressBoth();
            await transport.close();
        }
    });

    [
        prepareExpectedSignature('B2BF27F105D0E0E12F8BC913C8E124B2138E711AFAEAA7E85F186C2D8387F446', '1f22d8dd7df3051b0bc864763fbfccb177b24ba19d019a32ed6728638c4912bce77381d6d438b7cc18963a97319eb199fbf5cf8192586c6d616bbefa824cbcbc1f'),
    ].forEach(input => {
        it(`should properly sign hash`, async function () {
            const transport = await Transport.open({ apduPort: 40000, buttonPort: 5000, automationPort: 5000 });
            try {
                const hive = new Hive(transport);

                // enable hash signing
                await speculosButtons.pressRight();
                await speculosButtons.pressRight();
                await speculosButtons.pressBoth();
                await speculosButtons.pressBoth();
                await speculosButtons.pressRight();
                await speculosButtons.pressBoth();

                const signingHashPromise = hive.signHash(input.hash, `48'/13'/0'/0'/0'`);

                // accept hash
                await speculosButtons.pressLeft();
                await speculosButtons.pressLeft();
                await speculosButtons.pressBoth();

                const signature = await signingHashPromise;
                expect(signature).to.be.equal(input.expectedSignature);

                // disable hash signing
                await speculosButtons.pressRight();
                await speculosButtons.pressRight();
                await speculosButtons.pressBoth();
                await speculosButtons.pressBoth();
                await speculosButtons.pressRight();
                await speculosButtons.pressBoth();

            } finally {
                await transport.close();
            }
        }).timeout(10000)
    })


})