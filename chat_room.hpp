#pragma once
#include <memory>
#include <set>

// Interface for chat participants to decouple ChatSession and ChatRoom
class ChatParticipant {
public:
  virtual ~ChatParticipant() = default;
  virtual void deliver(std::string_view message) = 0;
};

using chat_participant_ptr = std::shared_ptr<ChatParticipant>;

class ChatRoom {
public:
  void join(chat_participant_ptr participant);
  void leave(chat_participant_ptr participant);
  void broadcast(std::string_view message);

private:
  std::set<chat_participant_ptr> participants_;
};
