/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nocontentmove.h"

namespace KWin
{

KWIN_EFFECT_FACTORY(NoContentMoveEffectFactory,
                    NoContentMoveEffect,
                    "metadata.json")

} // namespace KWin

#include "main.moc"
