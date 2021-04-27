#include "tnode.h"

TNode::TNode(std::string topicName,TNode *parent){
  name = topicName;
  if(parent)
    parentTopic = parent;
}

std::string TNode::getName(){
  return name;
}

std::vector<TNode> TNode::getTopics(){
  return directTopics;
}

TNode *TNode::addTopic(std::string topic){
  TNode *temp = new TNode(topic,this);
  directTopics.push_back(*temp);
  return temp;
}

void TNode::addMessage(std::string topic,char* msg,int retain,std::ofstream &output_log){
  int *isParent = 0; 
  TNode *msgNode = this->findTNode(topic,isParent);
  if((msgNode != nullptr) && (*isParent == 0)){
    if(retain)
      messages.push_back(msg);
    for(int i=0;i<subscribers.size();i++){
      int n;
      if(subscribers.at(i) != -1){
	if((n = write(subscribers.at(i),msg,sizeof(msg))) < 0)
	  output_log << "Write Failure\n";
	output_log<<"Writing to Client" << subscribers.at(i) << " with message " << topic << "\n\n";
	output_log.flush();
      }
    }
    return;
  }

  else if(*isParent){
    int delim = (int)topic.find_last_of("/");
    std::string name = topic.substr(delim+1,topic.size());
    TNode *temp = new TNode(name,msgNode);
    directTopics.push_back(*temp);
    if(retain)
      messages.push_back(msg);
  }
  return;
}

TNode *TNode::findTNode(std::string name,int *isParent){
  if(this->getName() == name)
    return this;
  
  int delim = (int)name.find_first_of("/");
  std::string topic;
  if(delim != -1)
    topic = name.substr(0,delim);
  else{
    if(directTopics.empty()){
      *isParent = 1;
      return parentTopic;
    }
    
    for(int i=0;i<directTopics.size();i++)
      if(directTopics.at(i).getName() == name)
	return &directTopics.at(i);

    *isParent = 1;
    return parentTopic;
  }
  
  for(int i=0;i<directTopics.size();i++){
    if(directTopics.at(i).getName() == topic)
      return directTopics.at(i).findTNode(name.substr(delim+1,name.size()),isParent);
  }

  return nullptr;
}

void TNode::addSubscriber(std::string topic,int sub){
  int *isParent = 0;
  TNode *temp = this->findTNode(topic,isParent);
  if(temp != nullptr && *isParent == 0)
    temp->directAddSubscriber(sub);
  else if(*isParent == 1){
    TNode *newTopic = temp->addTopic(topic);
    newTopic->directAddSubscriber(sub);
  }
  return;
}

void TNode::directAddSubscriber(int sub){
  subscribers.push_back(sub);
}

void TNode::removeSubscriber(std::string topic,int sub){
  int *isParent = 0;
  TNode *temp = this->findTNode(topic,isParent);
  if(temp != nullptr && *isParent == 0)
    temp->directRemoveSubscriber(sub);
  return;
}

void TNode::directRemoveSubscriber(int sub){
  int i;
  for(i=0;i<subscribers.size();i++){
    if(subscribers.at(i) == sub)
      break;
  }
  subscribers.at(i) = -1;
}
