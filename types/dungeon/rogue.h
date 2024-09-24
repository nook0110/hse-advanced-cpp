#pragma once

#include "dungeon.h"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <set>

class DungeonSolver {
public:
    explicit DungeonSolver(Room* starting_room) {
        CheckRoom(starting_room);

        while (!door_queue_.empty()) {
            auto next_door = door_queue_.front();
            door_queue_.pop();

            TryOpeningDoor(next_door);

            if (next_door->IsOpen()) {
                auto next_room = next_door->GoThrough();
                if (visited_[next_room]) {
                    continue;
                }
                if (CheckRoom(next_room)) {
                    return;
                }
            }
        }
    }
    Room* GetFinalRoom() const {
        return final_room_;
    }

private:
    void TryOpeningDoor(Door* door) const {
        for (const auto& key : keys_) {
            if (door->IsOpen()) {
                break;
            }
            door->TryOpen(key);
        }
    }

    void AddKeys(const Room* room) {
        size_t keys_size = keys_.size();
        for (size_t i = 0; i < room->NumKeys(); ++i) {
            keys_.insert(room->GetKey(i));
        }
        if (keys_size != keys_.size()) {
            UpdateClosedDoors();
        }
    }

    void UpdateClosedDoors() {
        for (auto door : closed_doors_) {
            TryOpeningDoor(door);
        }

        closed_doors_.remove_if([](const Door* door) { return door->IsOpen(); });
    }

    bool CheckRoom(Room* room) {
        visited_[room] = true;
        if (room->IsFinal()) {
            final_room_ = room;
            return true;
        }
        for (size_t i = 0; i < room->NumDoors(); ++i) {
            door_queue_.push(room->GetDoor(i));
        }
        AddKeys(room);
        return false;
    }

private:
    std::set<std::string, std::less<>> keys_;
    std::unordered_map<Room*, bool> visited_;
    Room* final_room_ = nullptr;
    std::queue<Door*> door_queue_;
    std::list<Door*> closed_doors_;
};

Room* FindFinalRoom(Room* starting_room) {
    return DungeonSolver(starting_room).GetFinalRoom();
}
