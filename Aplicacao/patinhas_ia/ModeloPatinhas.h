#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[4] = { 0 };
                        // tree #1
                        if (x[9] <= 9.985633611679077) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[9] <= 71.10407257080078) {
                                if (x[8] <= 0.3400295376777649) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #2
                        if (x[2] <= 1.1982499957084656) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[8] <= 0.32062068581581116) {
                                if (x[10] <= 13.019277334213257) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        // tree #3
                        if (x[2] <= 1.2785999774932861) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[8] <= 0.32062068581581116) {
                                if (x[6] <= 0.06324776168912649) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        // tree #4
                        if (x[2] <= 1.6082000136375427) {
                            if (x[7] <= 0.24023016542196274) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[6] <= 0.8699050545692444) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #5
                        if (x[11] <= 9.563614398241043) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[1] <= -0.12825000286102295) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[7] <= 0.24766789376735687) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[6] <= 0.853331059217453) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        // tree #6
                        if (x[1] <= -0.13865000009536743) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[7] <= 0.23739957809448242) {
                                if (x[9] <= 8.211409032344818) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        // tree #7
                        if (x[2] <= 1.6082000136375427) {
                            if (x[11] <= 61.94239616394043) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[8] <= 0.4891262799501419) {
                                    if (x[11] <= 161.6229705810547) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[1] <= -0.12825000286102295) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[6] <= 0.8007118999958038) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #8
                        if (x[2] <= 1.1982499957084656) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[9] <= 33.483110427856445) {
                                if (x[11] <= 9.486179009079933) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        // tree #9
                        if (x[8] <= 0.09849680587649345) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[2] <= 1.2785999774932861) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[8] <= 0.33828769624233246) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        // tree #10
                        if (x[9] <= 98.48829650878906) {
                            if (x[9] <= 36.91451454162598) {
                                if (x[9] <= 10.9360511302948) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #11
                        if (x[6] <= 0.7536004781723022) {
                            if (x[7] <= 0.06159634399227798) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[11] <= 48.72879123687744) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #12
                        if (x[11] <= 165.25702285766602) {
                            if (x[6] <= 0.25792718678712845) {
                                if (x[6] <= 0.05419549811631441) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[10] <= 93.42561340332031) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #13
                        if (x[2] <= 1.6069999933242798) {
                            if (x[7] <= 0.25711367279291153) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[10] <= 87.3545036315918) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #14
                        if (x[8] <= 0.09545928821898997) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[1] <= -0.2528499960899353) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[8] <= 0.3255252540111542) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        // tree #15
                        if (x[11] <= 171.9249382019043) {
                            if (x[1] <= 0.12440000101923943) {
                                if (x[2] <= 1.2785999774932861) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[7] <= 0.24023016542196274) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 0.0638375012204051) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[9] <= 41.96693229675293) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #16
                        if (x[10] <= 38.32956123352051) {
                            if (x[6] <= 0.05419549811631441) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[11] <= 162.13888931274414) {
                                votes[2] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #17
                        if (x[9] <= 75.05203628540039) {
                            if (x[8] <= 0.32062068581581116) {
                                if (x[7] <= 0.05965843622107059) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #18
                        if (x[9] <= 68.2844295501709) {
                            if (x[8] <= 0.32062068581581116) {
                                if (x[8] <= 0.09907674929127097) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #19
                        if (x[10] <= 83.47549819946289) {
                            if (x[8] <= 0.32062068581581116) {
                                if (x[10] <= 13.019277334213257) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #20
                        if (x[6] <= 0.853331059217453) {
                            if (x[9] <= 33.483110427856445) {
                                if (x[7] <= 0.06159634399227798) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 1.2785999774932861) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // tree #21
                        if (x[7] <= 0.07307104673236609) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[5] <= -22.6326003074646) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[2] <= 1.3081499934196472) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[11] <= 48.72879123687744) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }

                        // tree #22
                        if (x[11] <= 9.563614398241043) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[11] <= 175.0097198486328) {
                                if (x[7] <= 0.2011697068810463) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #23
                        if (x[10] <= 13.019277334213257) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[8] <= 0.33828769624233246) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[2] <= 1.2785999774932861) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        // tree #24
                        if (x[1] <= -0.13865000009536743) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[6] <= 0.25792718678712845) {
                                if (x[6] <= 0.0625772112980485) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        // tree #25
                        if (x[7] <= 0.23078438639640808) {
                            if (x[2] <= 1.6069999933242798) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[10] <= 103.23556518554688) {
                                if (x[0] <= 0.5200999975204468) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #26
                        if (x[9] <= 9.781526029109955) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[0] <= 0.2685000002384186) {
                                if (x[10] <= 39.433053970336914) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                if (x[9] <= 75.05203628540039) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #27
                        if (x[11] <= 9.563614398241043) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[6] <= 0.853331059217453) {
                                if (x[6] <= 0.25792718678712845) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #28
                        if (x[10] <= 11.860593557357788) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[6] <= 0.853331059217453) {
                                if (x[2] <= 1.556700050830841) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        // tree #29
                        if (x[11] <= 59.997297286987305) {
                            if (x[6] <= 0.06324776168912649) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[8] <= 0.5197662115097046) {
                                if (x[1] <= -0.34130001068115234) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                if (x[11] <= 161.68780517578125) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        // tree #30
                        if (x[10] <= 87.3545036315918) {
                            if (x[2] <= 1.5525500178337097) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[6] <= 0.05478523764759302) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 4; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                    /**
                    * Predict readable class name
                    */
                    const char* predictLabel(float *x) {
                        return idxToLabel(predict(x));
                    }

                    /**
                    * Convert class idx to readable name
                    */
                    const char* idxToLabel(uint8_t classIdx) {
                        switch (classIdx) {
                            case 0:
                            return "parado";
                            case 1:
                            return "andando";
                            case 2:
                            return "correndo";
                            case 3:
                            return "se_cocando";
                            default:
                            return "Houston we have a problem";
                        }
                    }

                protected:
                };
            }
        }
    }