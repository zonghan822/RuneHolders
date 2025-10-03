#pragma once


// stds
#include <unordered_map>


namespace Utility
{
	template <typename Key, typename Value,
			  typename Hasher = std::hash<Key>,
			  typename KeyEq = std::equal_to<Key>>
	class EnoMap
	{
	public:
		
		// Default map functions
		std::optional<Value> find(const Key& key) const
		{
			auto it = map_.find(key);
			if (it == map_.end())
				return std::nullopt;

			return it->second;
		}

		void insert(const Key& key, const Value& value) { map_[key] = value; }

		size_t size() const { return map_.size(); }

		// Overloads
		Value& operator[](const Key& key) { return map_[key]; }
		const Value& operator[](const Key& key) const { return map_.at(key); }


	private:
		std::unordered_map<Key, Value, Hasher, KeyEq> map_;
	};
}