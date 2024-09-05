#pragma once

#include <ranges>

namespace ranges = std::ranges;

namespace framework {
  namespace detail {
    // Type acts as a tag to find the correct operator| overload
    template <typename C> struct to_helper {};

    // This actually does the work
    template <typename Container, ranges::range R>
      requires std::
        convertible_to<ranges::range_value_t<R>, typename Container::value_type>
      Container operator|(R &&r, to_helper<Container>) {
      return Container{r.begin(), r.end()};
    }
  }

  // Couldn't find an concept for container, however a
  // container is a range, but not a view.
  /// Converts a range to a container
  template <ranges::range Container>
    requires(!ranges::view<Container>)
  auto to() {
    return detail::to_helper<Container>{};
  }
}
