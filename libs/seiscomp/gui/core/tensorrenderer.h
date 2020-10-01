/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#ifndef SEISCOMP_GUI_CORE_TENSORRENDERER_H
#define SEISCOMP_GUI_CORE_TENSORRENDERER_H


#include <QImage>
#include <QColor>

#ifndef Q_MOC_RUN
#include <seiscomp/math/matrix3.h>
#include <seiscomp/math/tensor.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API TensorRenderer {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		TensorRenderer();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void setShadingEnabled(bool);
		bool isShadingEnabled() const { return _shading; }

		void setMaterial(float ambient, float diffuse);

		void setMargin(int);
		void setProjectMargin(int);

		void setBorderColor(QColor);
		void setTColor(QColor);
		void setPColor(QColor);

		QPoint project(Math::Vector3f &v) const;
		QPoint project(Math::Vector3d &v) const;
		QPoint project(double azimuth, double dist = 1.0) const;
		QPoint projectMargin(double azimuth, int margin, double dist = 1.0) const;

		// Unprojects a point in screen coordinates. Returns false if the
		// point is not on the sphere (x,y is then valid on the plane and
		// z is undefined), true otherwise
		bool unproject(Math::Vector3d &v, const QPointF &p) const;

		void renderBorder(QImage& img);

		// strike, dip and slip are expected to be in degrees
		void render(QImage& img, double strike, double dip, double slip);
		void render(QImage& img, const Math::Matrix3f &m);
		void render(QImage& img, const Math::Matrix3d &m);
		void render(QImage& img, const Math::Tensor2Sd &t, const Math::Matrix3f &m);
		void render(QImage& img, const Math::Tensor2Sd &t);
		void renderNP(QImage& img, double strike, double dip, double slip, QColor color);


	private:
		QColor _colorT;
		QColor _colorP;
		QColor _border;

		bool   _shading;
		QPoint _center;
		int    _radius;
		int    _ballRadius;
		int    _projectRadius;
		int    _margin;
		int    _projectMargin;
		float  _materialAmbient;
		float  _materialDiffuse;
};


}
}

#endif
