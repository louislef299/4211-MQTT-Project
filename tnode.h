#ifndef TOPIC_NODE_H
#define TOPIC_NODE_H

#include "socket.h"

class TNode{
public:
  TNode(std::string name,TNode *parent = nullptr);

  std::string getName();

  std::vector<TNode> getTopics();

  TNode *addTopic(std::string topic);
  
  void addMessage(std::string topic,char* msg,int retain,std::ofstream &output_log);

  TNode *findTNode(std::string name,int *isParent);
  
  void addSubscriber(std::string topic,int sub);

  void directAddSubscriber(int sub);

  void removeSubscriber(std::string topic,int sub);
  
  void directRemoveSubscriber(int sub);
  
 private:
  std::string name;
  std::vector<TNode> directTopics;
  TNode *parentTopic;
  std::vector<char*> messages;
  std::vector<int> subscribers;
};

#endif // TOPIC_NODE_H
