#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace ecfw
{

    template <typename E>
    concept Entity = std::disjunction_v<
        std::is_same<E, std::uint16_t>,
        std::is_same<E, std::uint32_t>,
        std::is_same<E, std::uint64_t>
    >;

    template <Entity E>
    struct entity_traits;

    template <>
    struct entity_traits<std::uint64_t> {
        using entity_type  = std::uint64_t;
        using index_type   = std::uint32_t;
        using version_type = std::uint32_t;

        static constexpr entity_type index_mask = 0xFFFFFFFFu;
        static constexpr std::size_t index_size = 32u;

        /**
         * @brief Returns an entity's array index.
         * 
         * @param e The entity to find the index of.
         * @return An entity's array index.
         */
        static index_type index(entity_type e) noexcept {
            return e & index_mask;
        } 

        /**
         * @brief Returns an entity's index version.
         * 
         * This value represents how many previous times the index was used.
         * 
         * @param e The entity to find the version of.
         * @return An entity's index version.
         */
        static version_type version(entity_type e) noexcept {
            return e >> index_size;
        }

        /**
         * @brief Returns whether or not an an entity can be recycled.
         * 
         * @param e The entity to check.
         * @return true If the entity's version can be incremented.
         * @return false If the entity's version cannot be incremented.
         */
        static bool is_recyclable(entity_type e) noexcept {
            return version(e) < 0xFFFFFFFFu;
        }

        /**
         * @brief Combines an index and a version to create a new entity.
         * 
         * @param i The entity's index.
         * @param v The entity's version.
         * @return A newly created entity.
         */
        static entity_type combine(index_type i, version_type v) noexcept {
            return entity_type(v) << entity_type(index_size) | entity_type(i);
        }
    };

    template <>
    struct entity_traits<std::uint32_t> {
        using entity_type  = std::uint32_t;
        using index_type   = std::uint32_t;
        using version_type = std::uint32_t;

        static constexpr entity_type index_mask = 0xFFFFFu;
        static constexpr std::size_t index_size = 20u;

        /**
         * @brief Returns an entity's array index.
         * 
         * @param e The entity to find the index of.
         * @return An entity's array index.
         */
        static index_type index(entity_type e) noexcept {
            return e & index_mask;
        } 

        /**
         * @brief Returns an entity's index version.
         * 
         * This value represents how many previous times the index was used.
         * 
         * @param e The entity to find the version of.
         * @return An entity's index version.
         */
        static version_type version(entity_type e) noexcept {
            return e >> index_size;
        }

        /**
         * @brief Returns whether or not an an entity can be recycled.
         * 
         * @param e The entity to check.
         * @return true If the entity's version can be incremented.
         * @return false If the entity's version cannot be incremented.
         */
        static bool is_recyclable(entity_type e) noexcept {
            return version(e) < 0xFFFu;
        }

        /**
         * @brief Combines an index and a version to create a new entity.
         * 
         * @param i The entity's index.
         * @param v The entity's version.
         * @return A newly created entity.
         */
        static entity_type combine(index_type i, version_type v) noexcept {
            return entity_type(v) << entity_type(index_size) | entity_type(i);
        }
    };

    template <>
    struct entity_traits<std::uint16_t> {
        using entity_type  = std::uint16_t;
        using index_type   = std::uint16_t;
        using version_type = std::uint16_t;

        static constexpr entity_type index_mask = 0xFFFu;
        static constexpr std::size_t index_size = 12u;

        /**
         * @brief Returns an entity's array index.
         * 
         * @param e The entity to find the index of.
         * @return An entity's array index.
         */
        static index_type index(entity_type e) noexcept {
            return e & index_mask;
        } 

        /**
         * @brief Returns an entity's index version.
         * 
         * This value represents how many previous times the index was used.
         * 
         * @param e The entity to find the version of.
         * @return An entity's index version.
         */
        static version_type version(entity_type e) noexcept {
            return e >> index_size;
        }

        /**
         * @brief Returns whether or not an an entity can be recycled.
         * 
         * @param e The entity to check.
         * @return true If the entity's version can be incremented.
         * @return false If the entity's version cannot be incremented.
         */
        static bool is_recyclable(entity_type e) noexcept {
            return version(e) < 0xFu;
        }

        /**
         * @brief Combines an index and a version to create a new entity.
         * 
         * @param i The entity's index.
         * @param v The entity's version.
         * @return A newly created entity.
         */
        static entity_type combine(index_type i, version_type v) noexcept {
            return entity_type(v) << entity_type(index_size) | entity_type(i);
        }
    };

} // namespace ecfw