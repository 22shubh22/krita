/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_TOOL_FREEHAND_HELPER_H
#define __KIS_TOOL_FREEHAND_HELPER_H

#include <QObject>

#include "kis_types.h"
#include "kritaui_export.h"
#include <brushengine/kis_paint_information.h>
#include "kis_default_bounds.h"
#include <brushengine/kis_paintop_settings.h>
#include "kis_smoothing_options.h"
#include "strokes/freehand_stroke.h"

class KoPointerEvent;
class KoCanvasResourceManager;
class KisPaintingInformationBuilder;
class KisRecordingAdapter;
class KisStrokesFacade;
class KisPostExecutionUndoAdapter;
class KisPaintOp;


class KRITAUI_EXPORT KisToolFreehandHelper : public QObject
{
    Q_OBJECT

protected:

    typedef FreehandStrokeStrategy::PainterInfo PainterInfo;

public:

    KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                          const KUndo2MagicString &transactionText = KUndo2MagicString(),
                          KisRecordingAdapter *recordingAdapter = 0,
                          KisSmoothingOptions *smoothingOptions = 0);
    ~KisToolFreehandHelper() override;

    void setSmoothness(KisSmoothingOptionsSP smoothingOptions);
    KisSmoothingOptionsSP smoothingOptions() const;

    bool isRunning() const;

    void cursorMoved(const QPointF &cursorPos);

    /**
     * @param pixelCoords - The position of the KoPointerEvent, in pixel coordinates.
     */
    void initPaint(KoPointerEvent *event,
                   const QPointF &pixelCoords,
                   KoCanvasResourceManager *resourceManager,
                   KisImageWSP image,
                   KisNodeSP currentNode,
                   KisStrokesFacade *strokesFacade,
                   KisNodeSP overrideNode = 0,
                   KisDefaultBoundsBaseSP bounds = 0);
    void paintEvent(KoPointerEvent *event);
    void endPaint();

    QPainterPath paintOpOutline(const QPointF &savedCursorPos,
                                const KoPointerEvent *event,
                                const KisPaintOpSettingsSP globalSettings,
                                KisPaintOpSettings::OutlineMode mode) const;
    int canvasRotation();
    void setCanvasRotation(int rotation = 0);
    bool canvasMirroredH();
    void setCanvasHorizontalMirrorState (bool mirrored = false);
Q_SIGNALS:
    /**
     * The signal is emitted when the outline should be updated
     * explicitly by the tool. Used by Stabilizer option, because it
     * paints on internal timer events instead of the on every paint()
     * event
     */
    void requestExplicitUpdateOutline();

protected:
    void cancelPaint();
    int elapsedStrokeTime() const;

    void initPaintImpl(qreal startAngle,
                       const KisPaintInformation &pi,
                       KoCanvasResourceManager *resourceManager,
                       KisImageWSP image,
                       KisNodeSP node,
                       KisStrokesFacade *strokesFacade,
                       KisNodeSP overrideNode = 0,
                       KisDefaultBoundsBaseSP bounds = 0);

protected:

    virtual void createPainters(QVector<PainterInfo*> &painterInfos,
                                const KisDistanceInformation &startDist);

    // lo-level methods for painting primitives

    void paintAt(int painterInfoId, const KisPaintInformation &pi);

    void paintLine(int painterInfoId,
                   const KisPaintInformation &pi1,
                   const KisPaintInformation &pi2);

    void paintBezierCurve(int painterInfoId,
                          const KisPaintInformation &pi1,
                          const QPointF &control1,
                          const QPointF &control2,
                          const KisPaintInformation &pi2);

    // hi-level methods for painting primitives

    virtual void paintAt(const KisPaintInformation &pi);

    virtual void paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2);

    virtual void paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2);

private:
    void paint(KisPaintInformation &info);
    void paintBezierSegment(KisPaintInformation pi1, KisPaintInformation pi2,
                                                   QPointF tangent1, QPointF tangent2);

    void stabilizerStart(KisPaintInformation firstPaintInfo);
    void stabilizerEnd();
    KisPaintInformation getStabilizedPaintInfo(const QQueue<KisPaintInformation> &queue,
                                               const KisPaintInformation &lastPaintInfo);
    int computeAirbrushTimerInterval() const;

private Q_SLOTS:

    void finishStroke();
    void doAirbrushing();
    void doAsynchronousUpdate(bool forceUpdate = false);
    void stabilizerPollAndPaint();

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_TOOL_FREEHAND_HELPER_H */
