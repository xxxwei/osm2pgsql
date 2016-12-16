#ifndef OSMIUM_GEOM_TILE_HPP
#define OSMIUM_GEOM_TILE_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2016 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cassert>
#include <cstdint>

#include <osmium/geom/coordinates.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/osm/location.hpp>

namespace osmium {

    namespace geom {

        namespace detail {

            template <typename T>
            inline T restrict_to_range(T value, T min, T max) {
                if (value < min) return min;
                if (value > max) return max;
                return value;
            }

        } // namespace detail

        /**
         * A tile in the usual Mercator projection.
         */
        struct Tile {

            /// x coordinate
            uint32_t x;

            /// y coordinate
            uint32_t y;

            /// Zoom level
            uint32_t z;

            /**
             * Create a tile with the given zoom level and x any y tile
             * coordinates.
             *
             * The values are not checked for validity.
             *
             * @pre @code zoom <= 30 && x < 2^zoom && y < 2^zoom @endcode
             */
            explicit Tile(uint32_t zoom, uint32_t tx, uint32_t ty) noexcept : x(tx), y(ty), z(zoom) {
                assert(zoom <= 30u);
                assert(x < (1u << zoom));
                assert(y < (1u << zoom));
            }

            /**
             * Create a tile with the given zoom level that contains the given
             * location.
             *
             * The values are not checked for validity.
             *
             * @pre @code location.valid() && zoom <= 30 @endcode
             */
            explicit Tile(uint32_t zoom, const osmium::Location& location) :
                z(zoom) {
                assert(zoom <= 30u);
                assert(location.valid());
                const osmium::geom::Coordinates c = lonlat_to_mercator(location);
                const int32_t n = 1 << zoom;
                const double scale = detail::max_coordinate_epsg3857 * 2 / n;
                x = uint32_t(detail::restrict_to_range<int32_t>(int32_t((c.x + detail::max_coordinate_epsg3857) / scale), 0, n-1));
                y = uint32_t(detail::restrict_to_range<int32_t>(int32_t((detail::max_coordinate_epsg3857 - c.y) / scale), 0, n-1));
            }

            /**
             * Check whether this tile is valid. For a tile to be valid the
             * zoom level must be between 0 and 30 and the coordinates must
             * each be between 0 and 2^zoom-1.
             */
            bool valid() const noexcept {
                if (z > 30) {
                    return false;
                }
                const uint32_t max = 1 << z;
                return x < max && y < max;
            }

        }; // struct Tile

        /// Tiles are equal if all their attributes are equal.
        inline bool operator==(const Tile& lhs, const Tile& rhs) {
            return lhs.z == rhs.z && lhs.x == rhs.x && lhs.y == rhs.y;
        }

        inline bool operator!=(const Tile& lhs, const Tile& rhs) {
            return ! (lhs == rhs);
        }

        /**
         * This defines an arbitrary order on tiles for use in std::map etc.
         */
        inline bool operator<(const Tile& lhs, const Tile& rhs) {
            if (lhs.z < rhs.z) return true;
            if (lhs.z > rhs.z) return false;
            if (lhs.x < rhs.x) return true;
            if (lhs.x > rhs.x) return false;
            return lhs.y < rhs.y;
        }

    } // namespace geom

} // namespace osmium

#endif // OSMIUM_GEOM_TILE_HPP