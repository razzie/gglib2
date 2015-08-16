/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <locale>

namespace gg
{
    template<class T>
    int strcmpi(std::basic_string<T> s1, std::basic_string<T> s2,
                std::locale loc = std::locale())
    {
        if (s1.size() != s2.size()) return (s1.size() - s2.size());

        for (size_t i = 0; i < s1.size(); ++i)
        {
            int c1 = std::tolower(s1[i], loc);
            int c2 = std::tolower(s2[i], loc);
            if (c1 != c2) return (c1 - c2);
        }

        return 0;
    }

    template<class T>
    int strncmpi(std::basic_string<T> s1, std::basic_string<T> s2, size_t n,
                 std::locale loc = std::locale())
    {
        if (s1.size() < n || s2.size() < n) return (s1.size() - s2.size());

        for (size_t i = 0; i < n; ++i)
        {
            int c1 = std::tolower(s1[i], loc);
            int c2 = std::tolower(s2[i], loc);
            if (c1 != c2) return (c1 - c2);
        }

        return 0;
    }

    template<class T>
    std::basic_string<T> trim(std::basic_string<T> s,
                              std::locale loc = std::locale())
    {
        auto s_begin = s.begin(), s_end = s.end();
        auto it_first = s_end, it_last = s_end;

        for (auto it = s_begin; it != s_end; ++it)
        {
            if (!std::isspace(*it, loc))
            {
                if (it_first == s_end) it_first = it;
                it_last = it + 1;
            }
        }

        return std::basic_string<T>(it_first, it_last);
    }

	template<class T>
	void separate(const std::basic_string<T>& s,
				  std::vector<std::basic_string<T>>& elements,
				  char delimiter)
	{
		auto it1 = s.begin(), it2 = s.begin(), end = s.end();
		for (; it2 != end; ++it2)
		{
			if (*it2 == '\n')
			{
				if (it1 != it2) elements.emplace_back(it1, it2);
				it1 = it2 + 1;
			}
		}
		if (it1 == s.begin())
			elements.emplace_back(s);
		else
			elements.emplace_back(it1, end);
	}

    template<class FROM, class TO>
    std::basic_string<TO> convertString(std::basic_string<FROM> s,
                                        std::locale loc = std::locale())
    {
		std::basic_string<TO> result;
		result.reserve(s.size() + 1);

        FROM const* from_next;
        TO* to_next;
        mbstate_t state = {0};
		auto& facet = std::use_facet<std::codecvt<TO, FROM, std::mbstate_t> >(loc);
        std::codecvt_base::result conv_result = facet.in(state,
			&s[0], &s[s.size()], from_next,
			&result[0], &result[result.size()], to_next);

		assert(conv_result == std::codecvt_base::ok);
        *to_next = '\0';

        return result;
    }

    template<class T>
    bool isInteger(std::basic_string<T> s,
                   std::locale loc = std::locale())
    {
        if (s.empty()) return false;

        auto it = s.begin(), end = s.end();

        if (*it == '-') ++it;

        for (; it != end; ++it)
        {
            if (!std::isdigit(*it, loc)) return false;
        }

        return true;
    }

    template<class T>
    bool isFloat(std::basic_string<T> s,
                 std::locale loc = std::locale())
    {
        if (s.empty()) return false;

        auto it = s.begin(), end = s.end();
        bool point_used = false;

        if (*it == '-') ++it;

        for (; it != end; ++it)
        {
            if ((!std::isdigit(*it, loc) && *it != '.') ||
                (*it == '.' && point_used)) return false;

            if (*it == '.') point_used = true;
        }

        return true;
    }

    template<class T>
    bool isNumeric(std::basic_string<T> s,
                   std::locale loc = std::locale())
    {
        return (gg::isFloat(s, loc) || gg::isInteger(s, loc));
    }

    template<class T>
    bool containsSpace(std::basic_string<T> s,
                       std::locale loc = std::locale())
    {
        auto it = s.cbegin(), end = s.cend();
        bool found_char = false;
        bool found_space = false;

        for (; it != end; ++it)
        {
            if (!found_char && !std::isspace(*it, loc)) { found_char = true; continue; }
            if (found_char && !found_space && std::isspace(*it, loc)) { found_space = true; continue; }
            if (found_space && !std::isspace(*it, loc)) return true;
        }

        return false;
    }
};
