/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SelectionDecorator.h"

#include <KoShape.h>
#include <KoSelection.h>
#include <KoResourcePaths.h>
#include "kis_algebra_2d.h"

#include "kis_debug.h"
#include <KisHandlePainterHelper.h>
#include <KoCanvasResourceManager.h>
#include <KisQPainterStateSaver.h>
#include "KoShapeGradientHandles.h"

#include "kis_painting_tweaks.h"

#define HANDLE_DISTANCE 10

SelectionDecorator::SelectionDecorator(KoCanvasResourceManager *resourceManager)
    : m_hotPosition(KoFlake::Center)
    , m_handleRadius(7)
    , m_lineWidth(2)
    , m_showFillGradientHandles(false)
    , m_showStrokeFillGradientHandles(false)
{
    m_hotPosition =
        KoFlake::AnchorPosition(
            resourceManager->resource(KoFlake::HotPosition).toInt());
}

void SelectionDecorator::setSelection(KoSelection *selection)
{
    m_selection = selection;
}

void SelectionDecorator::setHandleRadius(int radius)
{
    m_handleRadius = radius;
    m_lineWidth = qMax(1, (int)(radius / 2));
}

void SelectionDecorator::setShowFillGradientHandles(bool value)
{
    m_showFillGradientHandles = value;
}

void SelectionDecorator::setShowStrokeFillGradientHandles(bool value)
{
    m_showStrokeFillGradientHandles = value;
}

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter)
{
    const bool haveOnlyOneEditableShape = m_selection->selectedEditableShapes().size() == 1;

    bool editable = false;

    QList<KoShape*> selectedShapes = m_selection->selectedShapes();
    if (selectedShapes.isEmpty()) return;

    foreach (KoShape *shape, KoShape::linearizeSubtree(selectedShapes)) {
        if (!haveOnlyOneEditableShape || !m_showStrokeFillGradientHandles) {
            KisHandlePainterHelper helper =
                KoShape::createHandlePainterHelper(&painter, shape, converter, m_handleRadius);

            helper.setHandleStyle(KisHandleStyle::secondarySelection());
            helper.drawRubberLine(shape->outlineRect());
        }

        if (!shape->isGeometryProtected()) {
            editable = true;
        }
    }

    const QRectF handleArea = m_selection->outlineRect();

    // draw extra rubber line around all the shapes
    if (selectedShapes.size() > 1) {
        KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelper(&painter, m_selection, converter, m_handleRadius);

        helper.setHandleStyle(KisHandleStyle::primarySelection());
        helper.drawRubberLine(handleArea);
    }

    // if we have no editable shape selected there
    // is no need drawing the selection handles
    if (editable) {
        KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelper(&painter, m_selection, converter, m_handleRadius);
        helper.setHandleStyle(KisHandleStyle::primarySelection());

        QPolygonF outline = handleArea;

        {
            helper.drawHandleRect(outline.value(0));
            helper.drawHandleRect(outline.value(1));
            helper.drawHandleRect(outline.value(2));
            helper.drawHandleRect(outline.value(3));
            helper.drawHandleRect(0.5 * (outline.value(0) + outline.value(1)));
            helper.drawHandleRect(0.5 * (outline.value(1) + outline.value(2)));
            helper.drawHandleRect(0.5 * (outline.value(2) + outline.value(3)));
            helper.drawHandleRect(0.5 * (outline.value(3) + outline.value(0)));

            QPointF hotPos = KoFlake::anchorToPoint(m_hotPosition, handleArea);
            helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
            helper.drawHandleRect(hotPos);
        }
    }

    if (haveOnlyOneEditableShape) {
        KoShape *shape = selectedShapes.first();

        if (m_showFillGradientHandles) {
            paintGradientHandles(shape, KoFlake::Fill, painter, converter);
        } else if (m_showStrokeFillGradientHandles) {
            paintGradientHandles(shape, KoFlake::StrokeFill, painter, converter);
        }
    }
}

void SelectionDecorator::paintGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant, QPainter &painter, const KoViewConverter &converter)
{
    KoShapeGradientHandles gradientHandles(fillVariant, shape);
    QVector<KoShapeGradientHandles::Handle> handles = gradientHandles.handles();

    KisHandlePainterHelper helper =
        KoShape::createHandlePainterHelper(&painter, shape, converter, m_handleRadius);

    const QTransform t = shape->absoluteTransformation(0).inverted();

    if (gradientHandles.type() == QGradient::LinearGradient) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(handles.size() == 2);

        if (handles.size() == 2) {
            helper.setHandleStyle(KisHandleStyle::gradientArrows());
            helper.drawGradientArrow(t.map(handles[0].pos), t.map(handles[1].pos), 1.5 * m_handleRadius);
        }
    }

    helper.setHandleStyle(KisHandleStyle::gradientHandles());

    Q_FOREACH (const KoShapeGradientHandles::Handle &h, handles) {
        if (h.type == KoShapeGradientHandles::Handle::RadialCenter) {
            helper.drawGradientCrossHandle(t.map(h.pos), 1.2 * m_handleRadius);
        } else {
            helper.drawGradientHandle(t.map(h.pos), 1.2 * m_handleRadius);
        }
    }
}
