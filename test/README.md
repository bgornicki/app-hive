# Hive Ledger app functional tests

## How to run functional tests

Install dependencies

```bash
npm ci
```

Run app within speculos emulator (you should build it first, instructions in main README file)

```bash
docker run --rm -it -v $(realpath .)/../bin:/speculos/apps -p 5000:5000 -p 40000:40000 speculos --display headless --apdu-port 40000 -s "salon stock memory business develop elegant chronic kite aspect nothing tone essay huge knock flip bar noise main cloth coin flavor only melody gain" apps/app.elf
```

Run tests

```
npm test
```
