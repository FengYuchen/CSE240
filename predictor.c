//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "YuchenFeng";
const char *studentID   = "A53220223";
const char *email       = "yuf114@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

//gshare data structure
uint8_t *gshareBHT;
uint32_t gshareHistory;

//tournament data structure
uint32_t *localPHT;
uint8_t *localBHT;
uint8_t *choicePT;
uint32_t globalHistory;
uint8_t *globalBHT;

//custom data structure;


void gshare_init() {
  gshareHistory = 0;
  gshareBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  memset(gshareBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
}

void gshare_train(uint32_t pc, uint8_t outcome) {
  //shift BHT prediction
  uint32_t gshareBHTIndex = (gshareHistory ^ pc) & ((1 << ghistoryBits) - 1);
  uint8_t previousPrediction = gshareBHT[gshareBHTIndex];
  if (outcome == NOTTAKEN) {
    if (previousPrediction != SN) {
      gshareBHT[gshareBHTIndex]--;
    }
  } else {
    if (previousPrediction != ST) {
      gshareBHT[gshareBHTIndex]++;
    }
  }
  //shift global history
  gshareHistory <<= 1;
  gshareHistory |= outcome;
}
uint8_t gshare_prediction(uint32_t pc) {
  uint32_t gshareBHTIndex = (gshareHistory ^ pc) & ((1 << ghistoryBits) - 1);
  uint8_t prediction = gshareBHT[gshareBHTIndex];
  uint8_t outcome = (prediction == WN || prediction == SN) ? NOTTAKEN : TAKEN;
  return outcome;
}
//tournament predictor
uint8_t local_prediction(uint32_t pc) {
    uint32_t localPHTIndex = pc & ((1 << pcIndexBits) - 1);
    uint32_t localBHTIndex = localPHT[localPHTIndex];
    uint8_t prediction = localBHT[localBHTIndex];
    uint8_t localOutcome = (prediction == WN || prediction == SN) ? NOTTAKEN : TAKEN;
    return localOutcome;
}

uint8_t global_prediction(uint32_t pc) {
    uint32_t globalBHTIndex = (globalHistory) & ((1 << ghistoryBits) - 1);
    uint8_t prediction = globalBHT[globalBHTIndex];
    uint8_t globalOutcome = ((prediction == WN || prediction == SN) ? NOTTAKEN : TAKEN);
    return globalOutcome;
}

void tournament_init() {
  localBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
  localPHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
  globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  choicePT = malloc((1 << ghistoryBits) * sizeof(uint8_t));

  memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
  memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
  memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
  memset(choicePT, WN, (1 << ghistoryBits) * sizeof(uint8_t));

  globalHistory = 0;
}


void tournament_train(uint32_t pc, uint8_t outcome) {
  uint8_t localOutcome = local_prediction(pc);
  uint8_t globalOutcome = global_prediction(pc);
  if (globalOutcome != localOutcome) {
    uint8_t choiceOutcome = localOutcome == outcome ? TAKEN : NOTTAKEN;
    if (choiceOutcome == NOTTAKEN) {
      if (choicePT[globalHistory] != SN) {
        choicePT[globalHistory]--;
      }
    } else {
      if (choicePT[globalHistory] != ST) {
        choicePT[globalHistory]++;
      }
    }
  }

  //train local predictor
  uint32_t localPHTIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t localBHTIndex = localPHT[localPHTIndex];
  if (outcome == NOTTAKEN) {
    if (localBHT[localBHTIndex] != SN) {
      localBHT[localBHTIndex]--;
    }
  } else {
    if (localBHT[localBHTIndex] != ST) {
      localBHT[localBHTIndex]++;
    }
  }
  localPHT[localPHTIndex] <<= 1;
  localPHT[localPHTIndex] &= ((1 << lhistoryBits) - 1);
  localPHT[localPHTIndex] |= outcome;

  //train global predictor
  if (outcome == NOTTAKEN) {
    if (globalBHT[globalHistory] != SN) {
      globalBHT[globalHistory]--;
    }
  } else {
    if (globalBHT[globalHistory] != ST) {
      globalBHT[globalHistory]++;
    }
  }
  globalHistory <<= 1;
  globalHistory &= ((1 << ghistoryBits) - 1);
  globalHistory |= outcome;
  return;
}

uint8_t tournament_prediction(uint32_t pc) {
  uint32_t globalBHTIndex = (globalHistory) & ((1 << ghistoryBits) - 1);
  uint32_t choiceOutcome = choicePT[globalBHTIndex];
  uint8_t globalOutcome = global_prediction(pc);
  uint8_t localOutcome = local_prediction(pc);

  if (choiceOutcome == WN || choiceOutcome == SN)     
    return globalOutcome;
  else 
    return localOutcome;
}
//custom predictor
void custom_init() {
  lhistoryBits = 10;
  pcIndexBits = 10;
  ghistoryBits = 12;
  localBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
  localPHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
  globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  choicePT = malloc((1 << ghistoryBits) * sizeof(uint8_t));

  memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
  memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
  memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
  memset(choicePT, WN, (1 << ghistoryBits) * sizeof(uint8_t));

  globalHistory = 0;
}

void custom_train(uint32_t pc, uint8_t outcome) {
  uint8_t localOutcome = local_prediction(pc);
  uint32_t globalBHTIndex = (globalHistory ^ pc) & ((1 << ghistoryBits) - 1);
  uint8_t prediction = globalBHT[globalBHTIndex];
  uint8_t globalOutcome = ((prediction == WN || prediction == SN) ? NOTTAKEN : TAKEN);
  if (globalOutcome != localOutcome) {
    uint8_t choiceOutcome = localOutcome == outcome ? TAKEN : NOTTAKEN;
    if (choiceOutcome == NOTTAKEN) {
      if (choicePT[globalHistory] != SN) {
        choicePT[globalHistory]--;
      }
    } else {
      if (choicePT[globalHistory] != ST) {
        choicePT[globalHistory]++;
      }
    }
  }
  //train local predictor
  uint32_t localPHTIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t localBHTIndex = localPHT[localPHTIndex];
  if (outcome == NOTTAKEN) {
    if (localBHT[localBHTIndex] != SN) {
      localBHT[localBHTIndex]--;
    }
  } else {
    if (localBHT[localBHTIndex] != ST) {
      localBHT[localBHTIndex]++;
    }
  }
  localPHT[localPHTIndex] <<= 1;
  localPHT[localPHTIndex] &= ((1 << lhistoryBits) - 1);
  localPHT[localPHTIndex] |= outcome;

  //train global predictor
  globalBHTIndex = (gshareHistory ^ pc) & ((1 << ghistoryBits) - 1);
  if (outcome == NOTTAKEN) {
    if (globalBHT[globalBHTIndex] != SN) {
      globalBHT[globalBHTIndex]--;
    }
  } else {
    if (globalBHT[globalBHTIndex] != ST) {
      globalBHT[globalBHTIndex]++;
    }
  }
  globalHistory <<= 1;
  globalHistory &= ((1 << ghistoryBits) - 1);
  globalHistory |= outcome;
  return;
}

uint8_t custom_prediction(uint32_t pc) {
  uint32_t choiceBHTIndex = (globalHistory) & ((1 << ghistoryBits) - 1);
  uint32_t choiceOutcome = choicePT[choiceBHTIndex];
  uint8_t localOutcome = local_prediction(pc);
  uint32_t globalBHTIndex = (globalHistory ^ pc) & ((1 << ghistoryBits) - 1);
  uint8_t prediction = globalBHT[globalBHTIndex];
  uint8_t globalOutcome = ((prediction == WN || prediction == SN) ? NOTTAKEN : TAKEN);
  if (choiceOutcome == WN || choiceOutcome == SN)     
    return globalOutcome;
  else 
    return localOutcome;
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
   switch (bpType) {
        case STATIC:
            return;
        case TOURNAMENT:
            tournament_init();
            break;
        case GSHARE:
            gshare_init();
            break;
        case CUSTOM:
            custom_init();
            break;
        default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_prediction(pc);
    case TOURNAMENT:
      return tournament_prediction(pc);
    case CUSTOM:
      return custom_prediction(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case TOURNAMENT:
      tournament_train(pc, outcome);
      break;
    case GSHARE:
      gshare_train(pc, outcome);
      break;
    case CUSTOM:
      custom_train(pc, outcome);
    default:
      break;
  }  
  return;
}
