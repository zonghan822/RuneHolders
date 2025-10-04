/*============================================================================================
* @file         eno_map.tpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration file for a map
*
* All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/


template <typename Key, typename Value, typename Hasher, typename KE>
std::optional<Value> EnoMap<Key, Value, Hasher, KE>::find(const Key& key) const
{
	auto it = map_.find(key);
	if (it == map_.end())
		return std::nullopt;

	return it->second;
}

template <typename Key, typename Value, typename Hasher, typename KE>
void EnoMap<Key, Value, Hasher, KE>::insert(const Key& key, const Value& value)
{
	map_[key] = value;
}

template <typename Key, typename Value, typename Hasher, typename KE>
size_t EnoMap<Key, Value, Hasher, KE>::size() const
{
	return map_.size();
}

template <typename Key, typename Value, typename Hasher, typename KE>
Value& EnoMap<Key, Value, Hasher, KE>::operator[](const Key& key)
{
	return map_[key];
}

template <typename Key, typename Value, typename Hasher, typename KE>
const Value& EnoMap<Key, Value, Hasher, KE>::operator[](const Key& key) const
{
	return map_.at(key);
}