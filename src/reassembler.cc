#include "reassembler.hh"

using namespace std;

Reassembler::Reassembler() : 
    unpoped_idx_(0), unassembled_idx_(0), unaccepted_idx_(0), need_close_(0), holes_({}) {}

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring, Writer &output)
{
    // Your code here.
    if (is_last_substring) this->need_close_ = true;
    uint64_t available_capacity = output.available_capacity();
	this->unaccepted_idx_ = available_capacity + this->unassembled_idx_;

    // 判断新到数据是否有价值
    if (unassembled_idx_ == unaccepted_idx_ || unassembled_idx_ >= first_index + data.size() || unaccepted_idx_ <= first_index) {
        if (this->need_close_ && !this->bytes_pending()) {
            output.close();
        }
        return;
    }
    // 判断是否覆盖this->unassembled_idx_所指地方
    uint64_t start = 0;
    if (this->unassembled_idx_ < first_index + data.size() && this->unassembled_idx_ > first_index) {
        start = this->unassembled_idx_ - first_index;
    }
    uint64_t end = data.size();
    if (first_index + data.size() > this->unaccepted_idx_) {
        end = this->unaccepted_idx_ - first_index;
    }
    segment seg = {
        start + first_index,
        data.substr(start, end),
    };

	// 使用迭代器遍历容器并查找符合条件的元素
	for (auto ite = holes_.begin(); ite != holes_.end();) {
		if (seg.merge(*ite)) {
			// 如果符合条件，将元素从原始容器中移除
			ite = holes_.erase(ite);
		} else {
			// 否则，继续下一个元素
			++ite;
		}
	}
	if (seg.idx == this->unassembled_idx_) { // 如果相等则需要发送
		if (available_capacity > seg.data.size()) {
			available_capacity = seg.data.size();
		}
		output.push(seg.data);
		this->unassembled_idx_ += available_capacity;
	} else {
		holes_.emplace_back(seg);
	}
	if (this->need_close_ && !this->bytes_pending()) {
		output.close();
	}
}

bool Reassembler::segment::merge(const Reassembler::segment& t) {
	uint64_t offset;
	if (idx >= t.idx && idx + data.size() <= t.idx + t.data.size() ) {
		idx = t.idx;
		data = std::move(t.data);
		return true;
	} else if (idx <= t.idx && t.idx + t.data.size() <= idx + data.size()) {
		return true;
	}
	else if (idx >= t.idx && t.idx + t.data.size() >= idx) {
		offset = t.idx + t.data.size();
		if (offset < idx + data.size()) {
			offset = idx + data.size();
			data = t.data + data.substr(t.data.size() - idx + t.idx, offset - t.idx - t.data.size());
		}
		idx = t.idx;
		return true;
	} else if (idx <= t.idx && idx + data.size() >= t.idx) {
		offset = idx + data.size();
		if (offset < t.idx + t.data.size()) {
			offset = t.idx + t.data.size();
			data.append(t.data.substr(data.size() - t.idx + idx, offset - data.size() - idx));
		}
		return true;
	} 
	
	
	return false;
}

uint64_t Reassembler::bytes_pending() const
{
	// Your code here.
	uint64_t ret = 0;
	for (auto ite = holes_.begin(); ite != holes_.end(); ite++) {
		ret += ite->data.size();
	}
	return ret;
}