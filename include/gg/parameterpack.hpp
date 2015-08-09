/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

namespace gg
{
	template<class... Params>
	class ParameterPack
	{
	public:
		template<size_t N>
		using Param = typename ParamImpl<N, Params...>;

		const size_t SIZE = sizeof...(Params);

	private:
		template<int N, class... T>
		struct ParamImpl;

		template<class T0, class... T>
		struct ParamImpl<0, T0, T...>
		{
			typedef T0 Type;
		};

		template<int N, class T0, class... T>
		struct ParamImpl<N, T0, T...>
		{
			typedef typename ParamImpl<N - 1, T...>::Type Type;
		};
	};
};
