#pragma once

#include <atomic>
#include <algorithm>

namespace zldsp::container {
    /**
     * an abstract FIFO that can be used by one producer and one consumer
     */
    class AbstractFIFO {
    public:
        struct Range {
            int start_index1;
            int block_size1;
            int start_index2;
            int block_size2;
        };

        explicit AbstractFIFO(const int capacity = 0)
            : capacity_(capacity),
              head_(0),
              tail_(0) {
        }

        ~AbstractFIFO() = default;

        void setCapacity(const int capacity) {
            capacity_ = capacity;
            head_.store(0);
            tail_.store(0);
        }

        int getCapacity() const { return capacity_; }

        /**
         * get the number of elements that can be read
         * @return
         */
        int getNumReady() const {
            const int current_head = head_.load(std::memory_order_relaxed);
            const int current_tail = tail_.load(std::memory_order_acquire);
            if (current_tail >= current_head) {
                return current_tail - current_head;
            }
            return capacity_ - current_head + current_tail;
        }

        /**
         * get the number of elements that can be written
         * @return
         */
        int getNumFree() const {
            const int current_head = head_.load(std::memory_order_acquire);
            const int current_tail = tail_.load(std::memory_order_relaxed);
            if (current_head > current_tail) {
                return current_head - current_tail - 1;
            }
            return capacity_ - current_tail + current_head - 1;
        }

        void prepareToWrite(const int num_to_write, Range &range) const {
            const int current_tail = tail_.load(std::memory_order_relaxed);

            range.block_size1 = std::min(num_to_write, capacity_ - current_tail);
            range.start_index1 = current_tail;

            if (num_to_write > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_write - range.block_size1;
            } else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
        }

        void finishedWrite(const int num_written) {
            if (num_written > 0) {
                const int current_tail = tail_.load(std::memory_order_relaxed);
                const int new_tail = (current_tail + num_written) % capacity_;
                tail_.store(new_tail, std::memory_order_release);
            }
        }

        void prepareToRead(const int num_to_read, Range &range) const {
            const int current_head = head_.load(std::memory_order_relaxed);

            range.block_size1 = std::min(num_to_read, capacity_ - current_head);
            range.start_index1 = current_head;

            if (num_to_read > range.block_size1) {
                range.start_index2 = 0;
                range.block_size2 = num_to_read - range.block_size1;
            } else {
                range.start_index2 = 0;
                range.block_size2 = 0;
            }
        }

        void finishedRead(const int num_read) {
            if (num_read > 0) {
                const int current_head = head_.load(std::memory_order_relaxed);
                const int new_head = (current_head + num_read) % capacity_;
                head_.store(new_head, std::memory_order_release);
            }
        }

    private:
        int capacity_;
        std::atomic<int> head_;
        std::atomic<int> tail_;
    };
}
