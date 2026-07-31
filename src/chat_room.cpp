#include "chat_room.hpp"
#include <print>

void ChatRoom::join(chat_participant_ptr participant) {
  participants_.insert(participant);

  std::println("[ROOM] New participant joined. Total active: {}",
               participants_.size());
}

void ChatRoom::leave(chat_participant_ptr participants) {
  participants_.erase(participants);

  std::println("[ROOM] Participant left. Total active: {}",
               participants_.size());
}

void ChatRoom::broadcast(std::string_view message) {
  for (const auto &participant : participants_) {
    participant->deliver(message);
  }
}
