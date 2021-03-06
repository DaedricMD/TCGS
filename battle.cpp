#include "battle.h"
void seed(){
  srand(time(0));
}
bool battle(Army* attacker, Army* defender){
  while(attacker!=NULL && defender!=NULL){
    vector<int> aRolls;
    vector<int> dRolls;
    int units,aLoss=0,bLoss=0;

    if(attacker->size() < defender->size()){
      units = attacker->size();
    }else{
      units = defender->size();
    }

    for(int i = 0; i < units; ++i){
      aRolls.push_back(rollDie());
    }
    for(int i = 0; i < units; ++i){
      dRolls.push_back(rollDie());
    }

    sort(aRolls.begin(), aRolls.end());
    reverse(aRolls.begin(),aRolls.end());

    sort(dRolls.begin(),dRolls.end());
    reverse(dRolls.begin(),dRolls.end());

    for(int i = 0;i<units;++i){
      if(aRolls[i] > dRolls[i]){
        bLoss++;
      }else{
        aLoss++;
      }
    }

    if(attacker->size() - aLoss <= 0){//if attacker loses more than it has then delete it
      delete attacker;
      if(defender->size() - bLoss <= 0){ //If both are reduced to 0 leave defender with 1 unit
        defender->eliminate(defender->size()-1);
        return false;
      }
      return false;
    }else{
      attacker->eliminate(aLoss);//otherwise reduce by loss
    }

    if(defender->size() - bLoss <= 0){//if defender loses more than it has then delete it
      delete defender;
      return true;
    }else{
      defender->eliminate(bLoss);//otherwise reduce by loss
    }

  }
}

int rollDie(){
  return ((rand()%20));
}
