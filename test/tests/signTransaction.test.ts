import Transport from '@ledgerhq/hw-transport-node-speculos';
import Hive from '@engrave/ledger-app-hive';
import { expect } from 'chai';
import { promises as fsPromises } from 'fs';
import * as speculosButtons from '../utils/speculosButtons';

const prepareExpectedSignature = (operation: string, expectedSignature: string) => ({
    operation, expectedSignature
})

describe('Sign transaction', async () => {

    [
        prepareExpectedSignature('vote', '1f0ecafb6491acc1aff07bec73dc9dad1b460f51803220b94e835ae8f6bcd18cdd2762dbb66241c3b03b475fa08c28a800e25bcdc13788e24509bd17527e09be84'),
        prepareExpectedSignature('comment', '1f71448ab4ac975daae15007065abc52a86898fba48faf144698c304d492ba4a8c5a6df3a7014ef451b8ddf69d6904225f8461bf2235cd4299094f1902699a5a9f'),
        prepareExpectedSignature('transfer', '1f5920665cc782f8351c52feb098b367da266dea9bd3f3a9b687b4b667bc84a2eb2c1905ab8227386742d96baf81605b981b0448ed4a9f81cecac35ad3f15275dd'),
        prepareExpectedSignature('transfer_to_vesting', '20367039018cdd8179822a58472f8bed9dfe9fa0401fd674ed8800f42cefaf71e2087bfc8aa9dc9dadb0e70079c243aeadccc4e9e8e5aa2f9698a7d8475d73a3b8'),
        prepareExpectedSignature('withdraw_vesting', '1f17facd89ae403ee6d2bb154512b8bf4ea2addffb26c49078eb993596f6bad3921046dcc92622374d08c577cff0d9349886ef414f73f8d6a3752ccdc540ac96c9'),
        prepareExpectedSignature('limit_order_create', '207936f3ebc23db6b4997a404dad9a7a47522c0b353b92c7c25a8e44f392c323d25c241e330624cd7efb6a2ea096c551c76b047f3ad0a44916a32acf50537d5fdd'),
        prepareExpectedSignature('limit_order_cancel', '1f60ddaaeee4b7e63f20744bfdff26ae72781a740efc8e36a73b41338c8f6fd987214f6dafcf8130e78e736fff918002d0fb767ab7e074c4378f95a8aed4026b85'),
        prepareExpectedSignature('feed_publish', '1f1b769e61faaad846af789cb0221bb5dd9ea0acf26fe379ed26ae2035a7505bcc52303e227031b1c576eecc4603e11df6e3c4902304b7332c3c8fbe3028a2a7c1'),
        prepareExpectedSignature('convert', '207463ec7308f71f3e0411c20bfc0bc7cbdc3c9b7ae9f89a4fff5f0b88af3e21eb03e48692a6ceaadf6fff34f64db49133f66cf76f21a3290ac8850ed2fd5ec545'),
        prepareExpectedSignature('account_create', '2076eec65df37a6b5915a1acc7dbf0c78e944ea3775defd25ca2b0f8c92dcc3acc5c6a5b89158cc8bc8ad028f71823e6882cd0602ef73c8ff92410b871957f57a9'),
        prepareExpectedSignature('account_update', '1f513f090e1bdb2cf7733d6f546e45a3a22b02da84c8ae79445998355a0158a00338817c3b7987d6b6f319b0365bb353cfe21257c45530397c819c84ea65b9de52'),
        prepareExpectedSignature('witness_update', '205cab743fbead0fac64f2390c807ff7ab0c730a5389d5981f6bd57835b93e3a53360b3b9161eb7235f99e7f4524dea6d4cc9ef596b596b4f315feeb80bc326e7d'),
        prepareExpectedSignature('account_witness_vote', '2079bd3738a76f2fbd303db6464936bd8b9c602857785d56457cc416ca3847113433e90882f1c55436d17da142dfb414c596cb443fd3a401bc8002ed2d24179903'),
        prepareExpectedSignature('account_witness_proxy', '2075a24d19d360896f4d8d6c38800cd0fd9d7194c2af2c8918489d325b47ddeaef2dc4fd15e03c5caecbf4eda5d14f3b06fe2d940125be90bb03622c437022672b'),
        prepareExpectedSignature('delete_comment', '206c51526bc2a97cd2fcd70706881b3122142e7f43e2fbfd4f32bbf45ec75b0af66ef2546522dfb01bac18c21d4d75506019c3c3acb019caad147b482e1e54bf67'),
        prepareExpectedSignature('custom_json', '1f4eab1e64b878d2a69ffccc132df420f5caf71e46a5b4a5b2c94aed4dc53dbde930b6f727d7241a09be6c15fbcc94b734e5ef000fd60d10acb1f57aa5f98ca803'),
        prepareExpectedSignature('comment_options', '205a5850d6dace3b168893bbcdc9e5ce8ec12f37deeaca366fa3dadb1a085303775a802c257bf4cbe6dd51ef5b7638b926e63a68e2c2cfc81f3b88844709b5c7a6'),
        prepareExpectedSignature('set_withdraw_vesting_route', '1f5519b506188a995c6f8a10e0bc1c299b55e99b189d32f3b2c025db895e62031e609a503296a791a436ee058aa5611469fa6811760003342d41fe2740c8a521df'),
        prepareExpectedSignature('claim_account', '1f6295c85bd6eff71cd3d279653c315841d77973985b6f7f455e7f89a24a1bf17c1e9621908f34ae5f25a1613040a6f2670d47c544619fdf23bfe288e1d023db43'),
        prepareExpectedSignature('create_claimed_account', '2003379f04a193d572293d974ea9bd10c268aa09c79225761d2625f8672f02b08e65aaa2665787ddee4d958cb152b3b8650dbbdf56a405ad442486a5eb01c1d37b'),
        prepareExpectedSignature('request_account_recovery', '1f146c40780bae972cda6b6d68dcb868361ec420666295ca7c6650eb006d993fb91292f3649e6f5499aac22f21cbdd941ec77c39d86da4892080a81995cdab4b5a'),
        prepareExpectedSignature('recover_account', '206d43fec3f16b23bf269744512e1c6a5e81043d6a44ab7d277449cd972a7f7e6b0d40ef62e234da2a31b63e903d49e91b5438c1ae188f55bdbc2b3fb3fd834bfb'),
        prepareExpectedSignature('change_recovery_account', '1f1d7152c4ce34898025ac1ff7b91a1c9a3a6b272c2ef85da2c307b441a30d02ea1e76a3b2566710f6d3ca9eab12a2c95d8cbac81137c56433bdb9119d93343f2f'),
        prepareExpectedSignature('transfer_to_savings', '20305ceab96a1aa3f3fdb9e3a168ca8779684079333fee1402ae473ef3f733ce47656caab3f9a8224e6202fd6114ef23b098fed01b9f3ab3b71d52792c04460fa4'),
        prepareExpectedSignature('transfer_from_savings', '2076b8983f051a41c4e1863a0dd4a11acdebc012a723731d7189d9ed4e380f74ca6efeb40d18d0d316be87a4e43104375c199c57c4f13dad6dae5b36d45068b9eb'),
        prepareExpectedSignature('cancel_transfer_from_savings', '1f6cd4f5a65a23b904de0558a49f9bb7e66a218f8c9f757e58c45ce4aef7bf53314bb2060c5f8e699532160fd97d77e641092d942924a4bff10f79b608123e8c8b'),
        prepareExpectedSignature('decline_voting_rights', '200aec8fd63dbc9bfa3f3658c2a23ac238c4d3c8b194eb8be0177b9a90a5008fe3476f7c5f61098943501b0e04e3a8495c40fa4f7ae74f400936704da289d6b978'),
        prepareExpectedSignature('reset_account', '1f30bfbef5db2b03cc89071a71214da867d544901b26faf9fa38d7ec87a4b564c02eaac567306a799718d56c9f12e445f6ef52de8139219c1cb9fc01b52f12cb85'),
        prepareExpectedSignature('set_reset_account', '1f3770b97bcbef22e58c7cef5471fff1d4ed6587a0fb71ba334b89696a2f96121b3bf1418eb965ecb4bf2e5d83db9297c60d9470292d83ac200b585825b06f109c'),
        prepareExpectedSignature('claim_reward_balance', '20248641aceb8ba441962559d069d049acde9acfc0b0203d874a624443ab03318a308ce5f1de5432fcede5b925e869ef891f0392eee0987f81b984f6e43a79d9ef'),
        prepareExpectedSignature('delegate_vesting_shares', '205cecbad305dbf38847b6cab04b2d05ac8b5cd4afb3ca4972d9f7ce11eacbe0a5271c31b151e43f53c1f14468993c0c647206999e9c5b8aeef92acfb78e72cad0'),
        prepareExpectedSignature('create_proposal', '1f741ec5bbda3fa814a697e613f74308dff51fb2d5e03fe79645b9d89e98110dc640b2abe32d9a493164910e9838fbb9821552f6fdef377dde1aa3ab28b0bbc969'),
        prepareExpectedSignature('update_proposal_votes', '1f176b9203a6f75e45ce3c4077030cc907a0a7b207c31931bf9ebede8524cdcbc4286843c00f125886da8fe1785ec04070e71daf5bf2822e7645473b064cabf3f5'),
        prepareExpectedSignature('remove_proposal', '203de03fd29dcfc90523bc0ed9a19ee120dfea9d63f6b4c9c43ed9f3ce7611c7c36ed918953743691e46a76cb6eb4e8c9b0f768a2e32f80b42af10bfb09a5e1e5d'),
        prepareExpectedSignature('update_proposal', '204e356db374ba31c988ab9bfa3d5ad905444dd2888e803a613bdeeba787a443c02988152a4f9011275ad87fc4e1a47d4763808883aa579c8ee9a2fab34f00449d'),
        prepareExpectedSignature('collateralized_convert', '20771cc128dbf1f8c841c826e1b29fa2de31e1bf3ec0e54b42e9f00b8df4aac6e90d2026c77fc383f70e433cfd05cbae9e6f83ffd6406b463882728c3c8adab6f6'),
        prepareExpectedSignature('recurrent_transfer', '1f6cb4225060272b497de0668db023945e2cc485af0a72c5e77062d707eaca0c627d1290f5d0a8f796de18656e8df8e96b473b83d4461bc92c390ba7668fb16204'),
    ].forEach(input => {
        it(`should properly sign transaction with ${input.operation} operation`, async function () {
            const tx = JSON.parse(await fsPromises.readFile(`./transactions/${input.operation}.json`, 'utf8'));
            const transport = await Transport.open({ apduPort: 40000, buttonPort: 5000, automationPort: 5000 });
            try {
                const hive = new Hive(transport);
                const signingTransactionPromise = hive.signTransaction(tx as any, `48'/13'/0'/0'/0'`);

                // accept transaction
                await speculosButtons.pressLeft();
                await speculosButtons.pressLeft();
                await speculosButtons.pressBoth();

                const { signatures } = await signingTransactionPromise;
                expect(signatures).to.be.instanceOf(Array).and.have.length(1);
                expect(signatures[0]).to.be.equal(input.expectedSignature);
            } finally {
                await transport.close();
            }
        })
    })


})