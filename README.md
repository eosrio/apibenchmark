# EOSIO API Benchmark
Nodeos Chain API benchmarking oracle contract

Made with ♥ by EOS Rio

EOS Mainnet: `apibenchmark`
BOS Mainnet: `apibenchmark`

### 1. Manual benchmarking instructions

Run the apibenchmark::cpu action against the api you want to benchmark, the .processed.elapsed param represents the contract execution time on the api.
```bash
cleos -u $API_URL push action apibenchmark cpu '[]' -p $ACCOUNT_NAME -j | jq -r ".processed.elapsed"
```

Save this value and report it along with other information from the node.

```bash
cleos push action apibenchmark report '{"tester":"$ACCOUNT_NAME","tests":[{"owner":"$NODE_OWNER","url":"$API_URL","status":200,"elapsed":$ELAPSED_TIME}]}' -p $ACCOUNT_NAME
```

### 2. Automated benchmarking scripts

Coming soon...
