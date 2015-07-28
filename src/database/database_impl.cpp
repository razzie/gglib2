/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include "database_impl.hpp"

static gg::DatabaseManager s_db;
gg::IDatabaseManager& gg::db = s_db;
