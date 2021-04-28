#include "tnode.h"

TNode::TNode(std::string topicName,TNode *parent){
  name = topicName;
  if(parent)
    parentTopic = parent;
}

std::string TNode::getName(){
  return name;
}

std::vector<TNode*> TNode::getTopics(){
  return directTopics;
}

TNode *TNode::addTopic(std::string topic){
  directTopics.push_back((new TNode(topic,this)));
  return directTopics.at(directTopics.size()-1);
}

void TNode::addMessage(std::string topic,char* msg,int retain,std::ofstream &output_log){ 
  TNode *msgNode = this->findTNode(topic);
  int isParent = this->isParent(topic,msgNode);
  if((msgNode != nullptr) && (isParent == 0)){
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

  else if(isParent){
    int delim = (int)topic.find_last_of("/");
    std::string name = topic.substr(delim+1,topic.size());
    directTopics.push_back((new TNode(name,msgNode)));
    if(retain)
      messages.push_back(msg);
  }
  return;
}

TNode *TNode::findTNode(std::string name){
  if(this->getName() == name)
    return this;
  
  int delim = (int)name.find_first_of("/");
  std::string topic;
  if(delim != -1)
    topic = name.substr(0,delim);
  else{
    if(directTopics.empty()){
      return parentTopic;
    }
    
    for(int i=0;i<directTopics.size();i++)
      if(directTopics.at(i)->getName() == name)
	return directTopics.at(i);

    return parentTopic;
  }
  
  for(int i=0;i<directTopics.size();i++){
    if(directTopics.at(i)->getName() == topic)
      return directTopics.at(i)->findTNode(name.substr(delim+1,name.size()));
  }

  return nullptr;
}

int TNode::addSubscriber(std::string topic,int sub){
  TNode *temp = this->findTNode(topic);
  int isParent = this->isParent(topic,temp);
  if(temp != nullptr && isParent == 0){
    temp->directAddSubscriber(sub);
    return 0;
  }
  else if(isParent == 1){
    TNode *newTopic = temp->addTopic(topic);
    newTopic->directAddSubscriber(sub);
    return 0;
  }
  return -1;
}

void TNode::directAddSubscriber(int sub){
  subscribers.push_back(sub);
}

int TNode::isParent(std::string topic,TNode *tempNode){
  int delim = (int)topic.find_last_of("/");
  std::string name;
  if(delim != -1)
    name = topic.substr(delim+1,topic.size());
  else
    name = topic;

  if(name == tempNode->getName())
    return 1;
  return 0;
}

int TNode::removeSubscriber(std::string topic,int sub){
  TNode *temp = this->findTNode(topic);
  int isParent = this->isParent(topic,temp);
  if(temp != nullptr && isParent == 0){
    temp->directRemoveSubscriber(sub);
    return 0;
  }
  return -1;
}

void TNode::directRemoveSubscriber(int sub){
  int i;
  for(i=0;i<subscribers.size();i++){
    if(subscribers.at(i) == sub)
      break;
  }
  subscribers.at(i) = -1;
}
