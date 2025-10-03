/*============================================================================================
* @file         eno_map.hpp
* @author       Lee Zhe An (zhean.l@digipen.edu)
* @date         10/3/2025
* @brief        Declaration/definition file for a bimap
*//*========================================================================================*/

#pragma once

// stds
#include <unordered_map>
#include <optional>
#include <utility>


namespace Utility
{
	template <typename Key, typename Value,
			  typename Hasher = std::hash<Key>,
			  typename KeyEq = std::equal_to<Key>>
	class EnoMap
	{
	public:
		/*============================================================================================
		 * \brief				Main reason for the custom map, instead of an iterator, we return
		 *						an optional value instead from finding
		 * \return				Value found from finding the key in the map
		*//*========================================================================================*/
		std::optional<Value> find(const Key&) const;

		/*============================================================================================
		 * \brief				Insert a key and value into the map
		 * \param				Key to insert
		 * \param				Value to insert
		*//*========================================================================================*/
		void insert(const Key&, const Value&);

		/*============================================================================================
		 * \brief				Returns the size of the map
		 * \return				Size of the map in size_t
		*//*========================================================================================*/
		size_t size() const;

		// Overloads
		/*============================================================================================
		 * \brief				Overloads on the subscript operator to retrieve associated value to key
		 * \param				Key to find
		 * \return				Value associated with passed in key
		*//*========================================================================================*/
		Value& operator[](const Key&);
		const Value& operator[](const Key&) const;


	private:
		std::unordered_map<Key, Value, Hasher, KeyEq> map_;
	};
	#include "eno_map.tpp"
}