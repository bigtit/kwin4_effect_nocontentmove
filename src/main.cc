/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nocontentmove.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(NoContentMoveEffect,
                                      "metadata.json",
                                      return NoContentMoveEffect::supported();,
                                      return false;)

} // namespace KWin

#include "main.moc"
