import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';

describe('Get app version', async () => {

    it('should get app version', async () => {
        const transport = await Transport.open({ apduPort: 40000 });
        try {
            const hive = new Hive(transport);
            const version = await hive.getAppVersion();
            expect(version).to.be.equal('1.1.0');
        } finally {
            await transport.close();
        }
    })
})