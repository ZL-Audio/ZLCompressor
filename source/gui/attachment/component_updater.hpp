// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <unordered_set>

#include "component_attachment.hpp"

namespace zlgui::attachment {
    class ComponentUpdater {
    public:
        ComponentUpdater() = default;

        ComponentUpdater(const ComponentUpdater&) = delete;
        ComponentUpdater& operator=(const ComponentUpdater&) = delete;

        void addAttachment(ComponentAttachment& attachment) {
            attachments_.insert(&attachment);
        }

        void removeAttachment(ComponentAttachment& attachment) {
            attachments_.erase(&attachment);
        }

        void updateComponents() {
            if (updater_flag_.exchange(false)) {
                for (auto& attachment : attachments_) {
                    attachment->updateComponent();
                }
            }
        }

        std::atomic<bool>& getFlag() { return updater_flag_; }

    private:
        std::unordered_set<ComponentAttachment*> attachments_;
        std::atomic<bool> updater_flag_{true};
    };
}
