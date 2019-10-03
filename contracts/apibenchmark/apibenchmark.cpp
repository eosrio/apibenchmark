#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <math.h>
#include <vector>
#pragma precision=log10l(ULLONG_MAX)/2
typedef enum { FALSE=0, TRUE=1 } BOOL;

// Max when calculating primes in cpu test
#define CPU_PRIME_MAX 375

#define N_TESTS 5

using namespace eosio;
using namespace std;

CONTRACT apibenchmark : public eosio::contract {
    public:
        using contract::contract;

        struct api_measurements {
            name owner;
            string url;
//            uint16_t status;
            uint16_t elapsed;
        };

        /**
         * Simple CPU benchmark that calculates Mersenne prime numbers.
         */
        [[eosio::action]] void cpu() {
            int p;
            //eosio::print_f("Mersenne primes:\n");
            for (p = 2; p <= CPU_PRIME_MAX; p += 1) {
                if (is_prime(p) && is_mersenne_prime(p)) {
                    // We need to keep an eye on this to make sure it doesn't get optimized out. So far so good.
                    //eosio::print_f(" %u", p);
                }
            }
        }

        [[eosio::action]] void rmreport(name producer) {
            require_auth(_self);
            api_index apis(_self, _self.value);
            auto itr = apis.find(owner.value);
            check(itr != apis.end(),"account not found");
            apis.erase(itr);
        }

        [[eosio::action]] void report(name tester,vector<api_measurements> tests) {
            require_auth(tester);

            tester_index testers(_self, _self.value);

            auto tester_itr = testers.find(tester.value);

            check(tester_itr != testers.end(),"Tester account not registered!");
            check(tests.size() > 0,"Must include at least one test");

            api_index apis(_self, _self.value);

            for (int i = 0; i < tests.size(); ++i) {

                api_measurements test = tests[i];
                auto itr = apis.find(test.owner.value);
                // measurement d {tester,test.elapsed,test.status,now()};
                measurement d {tester,test.elapsed,now()};

                if(itr != apis.end()) {

                    // existing owner
                    apis.modify(itr,_self,[&](auto& api){

                        int add = 0;

                        for(int j = 0; j < api.nodes.size(); ++j) {

                            if(api.nodes[j].url == test.url) {
                                
                                add = 1;

                                // add last test
                                api.nodes[j].measurements.push_back(d);

                                // delete first one if larger than limit
                                if(api.nodes[j].measurements.size() > N_TESTS) {
                                    api.nodes[j].measurements.erase(api.nodes[j].measurements.begin());
                                }

                                // take average
                                auto sum = 0;
                                for(int k = 0; k < api.nodes[j].measurements.size(); ++k) {
                                    //if(api.nodes[j].measurements[k].status == 200) {
                                        sum = sum + api.nodes[j].measurements[k].elapsed;
                                    //}
                                }
                                api.nodes[j].avg_perf = (uint16_t)((float)sum / (float)api.nodes[j].measurements.size());
                            }

                        }

                        if(add == 0) {
                            vector<measurement> m;
                            m.push_back(d);
                            node newnode {test.url,m,test.elapsed};
                            api.nodes.push_back(newnode);
                        }

                    });

                } else {

                    // new entry
                    apis.emplace(_self, [&](auto& api){
                        api.owner = test.owner;
                        vector<measurement> m;
                        m.push_back(d);
                        node n {test.url,m,test.elapsed};
                        api.nodes.push_back(n); 
                    });

                }

            }

            testers.modify(tester_itr,_self,[&](auto& t){
                t.reports = t.reports + tests.size();
            });

        }

        [[eosio::action]] void addtester(name owner) {
            require_auth(_self);
            tester_index testers(_self, _self.value);
            auto tester_itr = testers.find(owner.value);
            check(tester_itr == testers.end(),"Tester already registered!");
            testers.emplace(_self, [&](auto& tester){
                tester.owner = owner;
            });
        }

        [[eosio::action]] void rmtester(name owner) {
            require_auth(_self);
            tester_index testers(_self, _self.value);
            auto tester_itr = testers.find(owner.value);
            check(tester_itr != testers.end(),"Tester account not registered!");
            testers.erase(tester_itr);
        }

    private:

        struct measurement {
            name tester;
            uint16_t elapsed;
            // uint16_t status;
            uint32_t taken_at;
        };

        struct node {
            string url;
            vector<measurement> measurements;
            uint16_t avg_perf;
        };

        TABLE api {
            name owner;
            vector<node> nodes;4
            uint64_t primary_key() const {return owner.value;}
        };

        typedef multi_index<"apis"_n, api> api_index;

        TABLE tester {
            name owner;
            uint32_t reports;
            uint64_t primary_key() const {return owner.value;}
        };

        typedef multi_index<"testers"_n, tester> tester_index;

        BOOL is_prime(int p) {
            if (p == 2) {
                return TRUE;
            } else if (p <= 1 || p % 2 == 0) {
                return FALSE;
            }

            BOOL prime = TRUE;
            const int to = sqrt(p);
            int i;
            for (i = 3; i <= to; i += 2) {  
                if (!((prime = BOOL(p)) % i)) break;
            }
            return prime;
        }
      
        BOOL is_mersenne_prime(int p) {
            if (p == 2) return TRUE;

            const long long unsigned m_p = (1LLU << p) - 1;
            long long unsigned s = 4;
            int i;
            for (i = 3; i <= p; i++) {
                s = (s * s - 2) % m_p;
            }
            return BOOL(s == 0);
        }

        uint32_t now() {
            return current_time_point().sec_since_epoch();
        }
};

EOSIO_DISPATCH(apibenchmark, (cpu)(report)(addtester)(rmtester)(rmreport))
