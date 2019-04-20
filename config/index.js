let config = {
    NET_CONFIG: {
        blockchain: 'eos',
        protocol: 'https',
        host: 'api.kylin.alohaeos.com',
        port: 443,
        chainId: '5fff1dae8dc8e2fc4d5b23b2c7665c97f9e9d8edf2b6485a86ba311c25639191',
        private_key: "5JcQLXNJ4LJWiPN11pgwNTjqSP3mYiw9b9dxzQA4DeMZMZbimQe"
    },
    USE_EOSSERVICE:true,
    CONTRACTS: ["kmeal","kmealcoin"]
};


module.exports = config;
