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


#define SEISCOMP_COMPONENT Gui::OriginLocatorView

#include "originlocatorview.h"
#include "originlocatorview_p.h"

#include <seiscomp/gui/datamodel/ui_originlocatorview.h>
#include <seiscomp/gui/core/ui_diagramfilter.h>

#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/core/diagramwidget.h>
#include <seiscomp/gui/core/locator.h>
#include <seiscomp/gui/core/gradient.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/core/tensorrenderer.h>
#include <seiscomp/gui/datamodel/pickerview.h>
#include <seiscomp/gui/datamodel/importpicks.h>
#include <seiscomp/gui/datamodel/locatorsettings.h>
#include <seiscomp/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp/gui/datamodel/origindialog.h>
#include <seiscomp/gui/datamodel/utils.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/io/recordinput.h>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/momenttensor.h>
#include <seiscomp/datamodel/databasearchive.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/datamodel/journalentry.h>
#include <seiscomp/datamodel/publicobjectcache.h>
#include <seiscomp/seismology/regions.h>
#include <seiscomp/math/conversions.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/utils/misc.h>
#include <seiscomp/core/system.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/processing/magnitudeprocessor.h>

#include "ui_mergeorigins.h"
#include "ui_renamephases.h"
#include "ui_originlocatorview_commit.h"
#include "ui_originlocatorview_comment.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QToolTip>
#include <QWidget>

#include <algorithm>
#include <set>

#ifdef WIN32
#define snprintf _snprintf
#endif


#define SC_D (*_d_ptr)


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;


namespace {


struct CommitOptions {
	bool                         valid{false};
	bool                         forceEventAssociation{false};
	bool                         fixOrigin{false};
	bool                         returnToEventList{false};
	bool                         askForConfirmation{false};
	OPT(EventType)               eventType;
	OPT(EventTypeCertainty)      eventTypeCertainty;
	OPT(OPT(EvaluationStatus))   originStatus;
	OPT(string)                  magnitudeType;
	string                       eventName;
	string                       eventComment;

	struct OriginCommentProfile {
		string         id;
		string         value; // Output value
		string         label;
		vector<string> options;
		bool           allowFreeText{false};
	};

	vector<pair<string, string>> magnitudeTypes;
	vector<OriginCommentProfile> originCommentProfiles;

	void init(const std::string &prefix, Origin *origin) {
		try {
			forceEventAssociation = SCApp->configGetBool(prefix + "forceEventAssociation");
		}
		catch ( ... ) {}

		try {
			fixOrigin = SCApp->configGetBool(prefix + "fixOrigin");
		}
		catch ( ... ) {}

		try {
			returnToEventList = SCApp->configGetBool(prefix + "returnToEventList");
		}
		catch ( ... ) {}

		try {
			askForConfirmation = SCApp->configGetBool(prefix + "askForConfirmation");
		}
		catch ( ... ) {}

		try {
			auto profiles = SCApp->configGetStrings("olv.originComments");
			set<string> ids;

			for ( const auto &profile : profiles ) {
				CommitOptions::OriginCommentProfile commentProfile;

				try {
					commentProfile.id = SCApp->configGetString("olv.originComments." + profile + ".id");
					if ( commentProfile.id.empty() ) {
						SEISCOMP_WARNING("olv.originComments.%s.id is empty: ignoring",
						                 profile.c_str());
						continue;
					}

					if ( ids.find(commentProfile.id) != ids.end() ) {
						SEISCOMP_WARNING("Duplicate olv.originComments.%s.id: ignoring",
						                 profile.c_str());
						continue;
					}

					commentProfile.label = SCApp->configGetString("olv.originComments." + profile + ".label");
					if ( commentProfile.label.empty() ) {
						SEISCOMP_WARNING("olv.originComments.%s.label is empty: ignoring",
						                 profile.c_str());
						continue;
					}

					commentProfile.options = SCApp->configGetStrings("olv.originComments." + profile + ".options");
				}
				catch ( exception &e ) {
					SEISCOMP_WARNING("olv.originComments: %s, ignoring %s",
					                 e.what(), profile.c_str());
					continue;
				}

				try {
					commentProfile.allowFreeText = SCApp->configGetBool("olv.originComments." + profile + ".allowFreeText");
				}
				catch ( ... ) {}

				originCommentProfiles.push_back(commentProfile);
				ids.insert(commentProfile.id);
			}
		}
		catch ( ... ) {}

		setup(origin);
	}

	void setup(Origin *origin) {
		magnitudeTypes.clear();
		if ( origin ) {
			for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
				string value = "--";
				try {
					char buf[64];
					snprintf(buf, 63, "%.*f",
					         SCScheme.precision.magnitude,
					         origin->magnitude(i)->magnitude().value());
					value = buf;
				}
				catch ( ... ) {}
				magnitudeTypes.push_back(pair<string, string>(origin->magnitude(i)->type(), value));
			}
		}
		sort(magnitudeTypes.begin(), magnitudeTypes.end(), [](const pair<string, string> &i1, const pair<string, string> &i2) -> bool {
			return Core::compareNoCase(i1.first, i2.first) < 0;
		});

		for ( auto &profile : originCommentProfiles ) {
			profile.value = string();
			auto comment = origin ? origin->comment(profile.id) : nullptr;
			if ( comment ) {
				profile.value = comment->text();
			}
		}
	}
};


QString toString(const CommitOptions &opts) {
	QString s = QString(
		"Force event association: %1\n"
		"Fix origin: %2\n"
		"Return to list: %3\n"
		"Ask for confirmation: %4\n"
	)
	.arg(opts.forceEventAssociation ? "yes" : "no")
	.arg(opts.fixOrigin ? "yes" : "no")
	.arg(opts.returnToEventList ? "yes" : "no")
	.arg(opts.askForConfirmation ? "yes" : "no");

	if ( opts.eventType )
		s += QString("\nEvent type: %1").arg(opts.eventType->toString());
	if ( opts.eventTypeCertainty )
		s += QString("\nEvent type certainty: %1").arg(opts.eventTypeCertainty->toString());
	if ( opts.originStatus )
		s += QString("\nOrigin status: %1").arg((*opts.originStatus)->toString());
	if ( opts.magnitudeType ) {
		if ( !opts.magnitudeType->empty() )
			s += QString("\nMagnitude type: %1").arg(opts.magnitudeType->c_str());
		else
			s += QString("\nUnfix magnitude type");
	}
	if ( !opts.eventName.empty() )
		s += QString("\nEvent name: %1").arg(opts.eventName.c_str());
	if ( !opts.eventComment.empty() )
		s += QString("\nEvent comment: %1").arg(opts.eventComment.c_str());

	return s;
}

double diagramCeil(double value, double range) {
	if ( range == 0 ) {
		return ceil(value);
	}

	double pow10 = pow(0.1, floor(log10(abs(range) * 0.5)));
	return ceil(value * pow10) / pow10;
}

double diagramFloor(double value, double range) {
	if ( range == 0 ) {
		return floor(value);
	}

	double pow10 = pow(0.1, floor(log10(abs(range) * 0.5)));
	return floor(value * pow10) / pow10;
}

} // ns anonymous


namespace Seiscomp {
namespace Gui {

namespace {

const int UsedRole = Qt::UserRole + 1;
const int HoverRole = Qt::UserRole + 2;
const int RestoreRole = Qt::UserRole + 3;

MAKEENUM(
	ArrivalListColumns,
	EVALUES(
		USED,
		PUBLICID,
		STATUS,
		PHASE,
		WEIGHT,
		METHOD,
		POLARITY,
		ONSET,
		TAKEOFF,
		NETWORK,
		STATION,
		CHANNEL,
		RESIDUAL,
		DISTANCE,
		AZIMUTH,
		TIME,
		UNCERTAINTY,
		SLOWNESS,
		SLOWNESS_RESIDUAL,
		BACKAZIMUTH,
		BACKAZIMUTH_RESIDUAL,
		CREATED,
		LATENCY
	),
	ENAMES(
		"Used",
		"ID",
		"Status",
		"Phase",
		"Weight",
		"Method",
		"Polarity",
		"Onset",
		"Takeoff",
		"Net",
		"Sta",
		"Loc/Cha",
		"Timeres",
		"Dis",
		"Az",
		"Time",
		"+/-",
		"Slo",
		"Slores",
		"Baz",
		"Bazres",
		"Created",
		"Latency"
	)
);


MAKEENUM(
	PlotTabs,
	EVALUES(
		PT_DISTANCE,
		PT_AZIMUTH,
		PT_TRAVELTIME,
		PT_MOVEOUT,
		PT_POLAR,
		PT_FM
	),
	ENAMES(
		"Distance",
		"Azimuth",
		"TravelTime",
		"MoveOut",
		"Polar",
		"FirstMotion"
	)
);


MAKEENUM(
	PlotCols,
	EVALUES(
		PC_DISTANCE,
		PC_RESIDUAL,
		PC_TRAVELTIME,
		PC_AZIMUTH,
		PC_REDUCEDTRAVELTIME,
		PC_POLARITY,
		PC_FMAZI,
		PC_FMDIST
	),
	ENAMES(
		"Distance",
		"Residual",
		"TravelTime",
		"Azimuth",
		"MoveOut",
		"Polarity"
		"FMAzimuth",
		"FMDistance"
	)
);



QVariant colAligns[ArrivalListColumns::Quantity] = {
	QVariant(),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter)
};


bool colVisibility[ArrivalListColumns::Quantity] = {
	true,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false
};

inline int getMask(const QModelIndex &index) {
	int mask = 0;
	if ( index.sibling(index.row(), BACKAZIMUTH).data().isValid() )
		mask |= Seismology::LocatorInterface::F_BACKAZIMUTH;
	if ( index.sibling(index.row(), SLOWNESS).data().isValid() )
		mask |= Seismology::LocatorInterface::F_SLOWNESS;
	if ( index.sibling(index.row(), TIME).data().isValid() )
		mask |= Seismology::LocatorInterface::F_TIME;

	return mask;
}


void getRects(QList<QRect> &rects, const QStyleOptionViewItem &option,
              int labelWidth, int statusWidth, int spacing) {
	QStyle *style = qApp->style();
	int checkBoxWidth = style->subElementRect(QStyle::SE_CheckBoxIndicator,
	                                          &option).width();

	QRect statusRect = option.rect;
	statusRect.setWidth(statusWidth);

	QRect checkboxRect = statusRect;
	checkboxRect.translate(statusRect.width() + spacing, 0);
	checkboxRect.setWidth(checkBoxWidth);

	QRect rect = checkboxRect;
	rect.translate(checkboxRect.width() + spacing, 0);
	rect.setWidth(labelWidth);

	for ( int i = 0; i < 3; ++i ) {
		rects.push_back(rect);
		rect.translate(rect.width() + spacing, 0);
	}

	rects.append(statusRect);
	rects.append(checkboxRect);
}


class ArrivalsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		ArrivalsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( (left.column() == RESIDUAL && right.column() == RESIDUAL) ||
			     (left.column() == DISTANCE && right.column() == DISTANCE) ||
			     (left.column() == AZIMUTH && right.column() == AZIMUTH) ||
			     (left.column() == LATENCY && right.column() == LATENCY) ||
			     (left.column() == TAKEOFF && right.column() == TAKEOFF) ||
			     (left.column() == WEIGHT && right.column() == WEIGHT) )
				return sourceModel()->data(left, Qt::UserRole).toDouble() <
				       sourceModel()->data(right, Qt::UserRole).toDouble();
			else if ( left.column() == USED && right.column() == USED ) {
				return sourceModel()->data(left, UsedRole).toInt() <
				       sourceModel()->data(right, UsedRole).toInt();
			}
			else
				return QSortFilterProxyModel::lessThan(left, right);
		}
};


class RenamePhases : public QDialog {
	public:
		RenamePhases(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: QDialog(parent, f) {
			ui.setupUi(this);
		}

	public:
		Ui::RenamePhases ui;
};


class OriginCommitOptions : public QDialog {
	public:
		OriginCommitOptions(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: QDialog(parent, f) {
			ui.setupUi(this);

			QList<DataModel::EventType> eventTypesWhitelist;

			try {
				vector<string> eventTypes = SCApp->configGetStrings("olv.commonEventTypes");
				for ( size_t i = 0; i < eventTypes.size(); ++i ) {
					DataModel::EventType type;
					if ( !type.fromString(eventTypes[i].c_str()) ) {
						SEISCOMP_WARNING("olv.commonEventTypes: invalid type, ignoring: %s",
						                 eventTypes[i].c_str());
					}
					else
						eventTypesWhitelist.append(type);
				}
			}
			catch ( ... ) {}

			// Fill event types
			ui.comboEventTypes->addItem("- unset -");

			if ( eventTypesWhitelist.isEmpty() ) {
				QStringList types;
				for ( int i = int(EventType::First); i < int(EventType::Quantity); ++i ) {
					types << EventType::NameDispatcher::name(i);
				}

				types.sort();
				for ( int i = 1; i <= 4; ++i ) {
					ui.comboEventTypes->addItem(QString());
				}
				ui.comboEventTypes->insertSeparator(5);
				foreach (QString type, types) {
					if ( type == "earthquake" ) {
						ui.comboEventTypes->setItemText(1, type);
					}
					else if ( type == "not existing" ) {
						ui.comboEventTypes->setItemText(2, type);
					}
					else if ( type == "not locatable" ) {
						ui.comboEventTypes->setItemText(3, type);
					}
					else if ( type == "outside of network interest" ) {
						ui.comboEventTypes->setItemText(4, type);
					}
					else {
						ui.comboEventTypes->addItem(type);
					}
					ui.comboEventTypes->setItemData(ui.comboEventTypes->count()-1, type, Qt::ToolTipRole);
				}

			}
			else {
				QStringList types;
				bool usedFlags[DataModel::EventType::Quantity];
				for ( int i = 0; i < DataModel::EventType::Quantity; ++i )
					usedFlags[i] = false;

				for ( int i = 0; i < eventTypesWhitelist.count(); ++i ) {
					if ( usedFlags[eventTypesWhitelist[i]] ) continue;
					ui.comboEventTypes->addItem(eventTypesWhitelist[i].toString());
					usedFlags[eventTypesWhitelist[i]] = true;
				}
				ui.comboEventTypes->insertSeparator(ui.comboEventTypes->count()+1);

				QColor reducedColor;
				reducedColor = blend(palette().color(QPalette::Text), palette().color(QPalette::Base), 75);

				for ( int i = 0; i < DataModel::EventType::Quantity; ++i ) {
					if ( usedFlags[i] ) continue;
					types << EventType::NameDispatcher::name(i);
				}

				types.sort();
				foreach (QString type, types) {
					ui.comboEventTypes->addItem(type);
					ui.comboEventTypes->setItemData(ui.comboEventTypes->count()-1,
					                                reducedColor, Qt::ForegroundRole);
				}
			}

			// Increase number of visible items to 20 (default=10), ensure
			// all commonEventTypes are visible
			int maxItems = max(20, eventTypesWhitelist.size() + 1);
			ui.comboEventTypes->setMaxVisibleItems(maxItems);

			EventType defaultType = EARTHQUAKE;
			ui.comboEventTypes->setCurrentIndex(ui.comboEventTypes->findText(defaultType.toString()));

			// event certainty
			ui.comboEventTypeCertainty->addItem("- unset -");
			for ( int i = int(EventTypeCertainty::First); i < int(EventTypeCertainty::Quantity); ++i )
				ui.comboEventTypeCertainty->addItem(EventTypeCertainty::NameDispatcher::name(i));
			ui.comboEventTypeCertainty->setCurrentIndex(0);

			// origin status
			ui.comboOriginStates->addItem("- unset -");
			for ( int i = 0; i < 6; ++i ) {
				ui.comboOriginStates->addItem(QString());
			}
			for ( int i = int(EvaluationStatus::First); i < int(EvaluationStatus::Quantity); ++i ) {
				if ( EvaluationStatus::Type(i) == FINAL ) {
					ui.comboOriginStates->setItemText(1, EvaluationStatus::NameDispatcher::name(i));
				}
				else if ( EvaluationStatus::Type(i) == REVIEWED ) {
					ui.comboOriginStates->setItemText(2, EvaluationStatus::NameDispatcher::name(i));
				}
				else if ( EvaluationStatus::Type(i) == CONFIRMED ) {
					ui.comboOriginStates->setItemText(3, EvaluationStatus::NameDispatcher::name(i));
				}
				else if ( EvaluationStatus::Type(i) == PRELIMINARY ) {
					ui.comboOriginStates->setItemText(4, EvaluationStatus::NameDispatcher::name(i));
				}
				else if ( EvaluationStatus::Type(i) == REPORTED ) {
					ui.comboOriginStates->setItemText(5, EvaluationStatus::NameDispatcher::name(i));
				}
				else if ( EvaluationStatus::Type(i) == REJECTED ) {
					ui.comboOriginStates->setItemText(6, EvaluationStatus::NameDispatcher::name(i));
				}
				else {
					ui.comboOriginStates->addItem(EvaluationStatus::NameDispatcher::name(i));
				}
			}

			ui.comboEQComment->addItem("");
			ui.comboEQComment->setEditable(true);
			ui.comboEQComment->setVisible(false);
		}


		void setOptions(const CommitOptions &options, const Event *event, bool isLocalOrigin) {
			ui.cbAssociate->setChecked(options.forceEventAssociation);
			ui.cbFixSolution->setChecked(options.fixOrigin);

			ui.labelPreferredMagnitude->setVisible(false);
			ui.comboPreferredMagnitude->setVisible(false);

			ui.labelPreferredMagnitude->setVisible(true);
			ui.comboPreferredMagnitude->setVisible(true);
			ui.comboPreferredMagnitude->addItem(tr("- no changes -"));
			ui.comboPreferredMagnitude->addItem(tr("- automatic -"));
			ui.comboPreferredMagnitude->setCurrentIndex(0);
			for ( auto &type : options.magnitudeTypes ) {
				ui.comboPreferredMagnitude->addItem(
					QString("%1 (%2)").arg(type.first.data()).arg(type.second.data()),
					type.first.data()
				);
			}

			if ( options.magnitudeType ) {
				if ( options.magnitudeType->empty() ) {
					ui.comboPreferredMagnitude->setCurrentIndex(1);
				}
				else {
					int idx = ui.comboPreferredMagnitude->findData(options.magnitudeType->data());
					if ( idx > 1 ) {
						ui.comboPreferredMagnitude->setCurrentIndex(idx);
					}
					else {
						ui.comboPreferredMagnitude->addItem(
							QString("%1 (not available)").arg(options.magnitudeType->data()),
							options.magnitudeType->data()
						);
						ui.comboPreferredMagnitude->setCurrentIndex(ui.comboPreferredMagnitude->count() - 1);
					}
				}
			}

			try {
				commentOptions = SCApp->configGetStrings("olv.commit.eventCommentOptions");
				if ( !commentOptions.empty() ) {
					ui.editEQComment->setVisible(false);
					ui.comboEQComment->setVisible(true);

					for ( vector<string>::const_iterator it = commentOptions.begin();
					      it != commentOptions.end(); ++it ) {
						ui.comboEQComment->addItem(it->c_str());
						if ( options.eventComment == *it )
							ui.comboEQComment->setCurrentIndex(ui.comboEQComment->count() - 1);
					}
				}
			}
			catch ( ... ) {}

			ui.cbBackToEventList->setChecked(options.returnToEventList);

			if ( !event || !isLocalOrigin ) {
				ui.cbAssociate->setVisible(false);
				ui.cbAssociate->setEnabled(false);
			}

			if ( options.eventType ) {
				int idx = ui.comboEventTypes->findText(options.eventType->toString());
				if ( idx != -1 )
					ui.comboEventTypes->setCurrentIndex(idx);
			}

			if ( options.eventTypeCertainty ) {
				int idx = ui.comboEventTypeCertainty->findText(options.eventTypeCertainty->toString());
				if ( idx != -1 )
					ui.comboEventTypeCertainty->setCurrentIndex(idx);
			}

			// Populate earthquake name
			if ( !options.eventName.empty() )
				ui.editEQName->setText(options.eventName.c_str());

			// Fill operator's comment
			if ( !options.eventComment.empty() ) {
				if ( commentOptions.empty() ) {
					ui.editEQComment->setText(options.eventComment.c_str());
				}
				else {
					// search for current comment in list of available options,
					// add and select it in case it is not present
					int idx = -1, i = 1;
					for ( vector<string>::const_iterator it = commentOptions.begin();
					      it != commentOptions.end(); ++it, ++i ) {
						if ( *it == options.eventComment ) {
							idx = i;
							break;
						}
					}
					if ( idx < 0 ) {
						ui.comboEQComment->insertItem(1, options.eventComment.c_str());
						idx = 1;
					}
					ui.comboEQComment->setCurrentIndex(idx);
				}
			}

			if ( event ) {
				ui.cbAssociate->setText(QString(ui.cbAssociate->text()).arg(event->publicID().c_str()));
			}

			if ( options.originStatus && *options.originStatus ) {
				int idx = ui.comboOriginStates->findText((*options.originStatus)->toString());
				if ( idx != -1 )
					ui.comboOriginStates->setCurrentIndex(idx);
			}

			for ( auto comboBox : originComments ) {
				delete comboBox;
			}
			originComments.clear();

			for ( auto &profile : options.originCommentProfiles ) {
				ui.frameEventOptions->layout()->addWidget(new QLabel((profile.label + ":").c_str()));
				QComboBox *comboComment = new QComboBox;
				comboComment->setProperty("id", QString(profile.id.c_str()));
				comboComment->setEditable(profile.allowFreeText);
				comboComment->setToolTip(
					tr("Populates comment with id '%1'")
					.arg(profile.id.c_str())
				);
				for ( auto &option : profile.options ) {
					comboComment->addItem(option.c_str());
				}
				auto idx = comboComment->findText(profile.value.c_str());
				if ( idx >= 0 ) {
					comboComment->setCurrentIndex(idx);
				}
				else if ( profile.allowFreeText ) {
					comboComment->setEditText(profile.value.c_str());
				}
				originComments.append(comboComment);
				ui.frameEventOptions->layout()->addWidget(comboComment);
			}

			auto preferredSize = sizeHint();
			if ( preferredSize.width() < width() ) {
				preferredSize.setWidth(width());
			}
			if ( preferredSize.height() < height() ) {
				preferredSize.setHeight(height());
			}
			resize(preferredSize);
		}


		bool getOptions(CommitOptions &options) {
			options.forceEventAssociation = ui.cbAssociate->isEnabled() && ui.cbAssociate->isChecked();
			options.fixOrigin = ui.cbFixSolution->isChecked();
			options.returnToEventList = ui.cbBackToEventList->isChecked();

			if ( ui.comboEventTypes->currentIndex() > 0 ) {
				EventType type;
				if ( type.fromString(ui.comboEventTypes->currentText().toStdString()) )
					options.eventType = type;
				else {
					QMessageBox::critical(this, "Internal Error", "Invalid event type selected");
					return false;
				}
			}
			else {
				options.eventType = Core::None;
			}

			if ( ui.comboEventTypeCertainty->currentIndex() > 0 ) {
				EventTypeCertainty typeCertainty;
				if ( typeCertainty.fromString(ui.comboEventTypeCertainty->currentText().toStdString()) )
					options.eventTypeCertainty = typeCertainty;
				else {
					QMessageBox::critical(this, "Internal Error", "Invalid event type certainty selected");
					return false;
				}
			}
			else {
				options.eventTypeCertainty = Core::None;
			}

			if ( ui.comboOriginStates->currentIndex() > 0 ) {
				EvaluationStatus originStatus;
				if ( originStatus.fromString(ui.comboOriginStates->currentText().toStdString()) )
					options.originStatus = OPT(EvaluationStatus)(originStatus);
				else {
					QMessageBox::critical(this, "Internal Error", "Invalid origin evaluation status selected");
					return false;
				}
			}
			else {
				OPT(EvaluationStatus) none;
				options.originStatus = none;
			}

			options.eventName = ui.editEQName->text().toStdString();

			if ( commentOptions.empty() ) {
				options.eventComment = ui.editEQComment->toPlainText().toStdString();
			}
			else {
				options.eventComment = ui.comboEQComment->currentText().toStdString();
			}

			for ( auto comboBox : originComments ) {
				auto id = comboBox->property("id").toString().toStdString();
				for ( auto &profile : options.originCommentProfiles ) {
					if ( profile.id == id ) {
						profile.value = comboBox->currentText().toStdString();
						break;
					}
				}
			}

			return true;
		}


	public:
		Ui::OriginCommitOptions ui;
		vector<string>          commentOptions;
		QVector<QComboBox*>     originComments;
};


class CommentEdit : public QDialog {
	public:
		CommentEdit(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: QDialog(parent, f) {
			ui.setupUi(this);
		}

	public:
		Ui::OriginCommentOptions ui;
};


class NodalPlaneDialog : public QDialog {
	public:
		NodalPlaneDialog(QWidget *parent = 0) : QDialog(parent) {
			resize(184, 144);

			setWindowTitle("Nodal plane");

			QVBoxLayout *vboxLayout = new QVBoxLayout(this);
			QGridLayout *gridLayout = new QGridLayout();
			QLabel *label = new QLabel("Strike", this);
			QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
			sizePolicy.setHorizontalStretch(0);
			sizePolicy.setVerticalStretch(0);
			sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
			label->setSizePolicy(sizePolicy);

			gridLayout->addWidget(label, 0, 0, 1, 1);

			QLabel *label_2 = new QLabel("Dip", this);

			gridLayout->addWidget(label_2, 1, 0, 1, 1);

			QLabel *label_3 = new QLabel("Rake", this);

			gridLayout->addWidget(label_3, 2, 0, 1, 1);

			sbStrike = new QSpinBox(this);
			sbStrike->setSuffix(" deg");
			sbStrike->setObjectName(QString::fromUtf8("sbStrike"));
			sbStrike->setAlignment(Qt::AlignRight);
			sbStrike->setMaximum(360);
			sbStrike->setMinimum(-360);

			gridLayout->addWidget(sbStrike, 0, 1, 1, 1);

			sbDip = new QSpinBox(this);
			sbDip->setSuffix(" deg");
			sbDip->setObjectName(QString::fromUtf8("sbDip"));
			sbDip->setAlignment(Qt::AlignRight);
			sbDip->setMaximum(360);
			sbDip->setMinimum(-360);

			gridLayout->addWidget(sbDip, 1, 1, 1, 1);

			sbRake = new QSpinBox(this);
			sbRake->setSuffix(" deg");
			sbRake->setObjectName(QString::fromUtf8("sbRake"));
			sbRake->setAlignment(Qt::AlignRight);
			sbRake->setMaximum(360);
			sbRake->setMinimum(-360);

			gridLayout->addWidget(sbRake, 2, 1, 1, 1);

			vboxLayout->addLayout(gridLayout);

			QSpacerItem *spacerItem = new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

			vboxLayout->addItem(spacerItem);

			QHBoxLayout *hboxLayout = new QHBoxLayout();
			QSpacerItem *spacerItem1 = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

			hboxLayout->addItem(spacerItem1);

			QPushButton *okButton = new QPushButton(this);
			okButton->setText(QApplication::translate("", "OK", 0));

			hboxLayout->addWidget(okButton);

			QPushButton *cancelButton = new QPushButton(this);
			cancelButton->setText(QApplication::translate("", "Cancel", 0));

			hboxLayout->addWidget(cancelButton);

			vboxLayout->addLayout(hboxLayout);

			QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
			QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		}

	public:
		QSpinBox *sbStrike;
		QSpinBox *sbDip;
		QSpinBox *sbRake;
};


QPointF equalarea(double azi, double dip) {
	dip = 90-dip;
	if ( dip > 90 ) {
		dip = 180 - dip;
		azi -= 180;
	}

	double z = sqrt(2)*sin(0.5*deg2rad(dip));
	return QPointF(z, azi);
}


class PlotWidget : public OriginLocatorPlot {
	public:
		enum ShapeType {
			ST_CIRCLE,
			ST_TRIANGLE,
			ST_TRIANGLE2,
			ST_RECT,
			ST_CROSS
		};

		enum PolarityType {
			POL_POSITIVE,
			POL_NEGATIVE,
			POL_UNDECIDABLE,
			POL_UNSET,
			POL_QUANTITY
		};

		struct Shape {
			Shape() : shown(false) {}

			Shape(ShapeType t, int s)
			: type(t), fillUsed(false), shown(true) {
				setSize(s);
			}

			Shape(ShapeType t, int s, const QBrush &f)
			: type(t), fillUsed(true), fill(f), shown(true) {
				setSize(s);
			}

			Shape(ShapeType t, int s, const QBrush &f, const QPen &st)
			: type(t), fillUsed(true), fill(f), stroke(st), shown(true) {
				setSize(s);
			}

			void setSize(int s) {
				size = s;

				if ( type == ST_TRIANGLE ) {
					poly = QPolygonF() << QPointF(-1,0.85)
									   << QPointF(1,0.85)
									   << QPointF(0,-0.85);
				}
				else if ( type == ST_TRIANGLE2 ) {
					poly = QPolygonF() << QPointF(1,-0.85)
									   << QPointF(-1,-0.85)
									   << QPointF(0,0.85);
				}
				else
					poly.clear();

				if ( !poly.isEmpty() ) {
					qreal hsize = size*0.5;
					for ( int i = 0; i < poly.size(); ++i )
						poly[i] *= hsize;
				}
			}


			void init(const string &param) {
				vector<string> toks;
				try {
					toks = SCApp->configGetStrings(param);
				}
				catch ( ... ) {
					return;
				}

				shown = true;
				poly.clear();

				if ( toks.size() > 0 ) {
					if ( toks[0] == "circle" )
						type = ST_CIRCLE;
					else if ( toks[0] == "triangle" )
						type = ST_TRIANGLE;
					else if ( toks[0] == "triangle2" )
						type = ST_TRIANGLE2;
					else if ( toks[0] == "rectangle" )
						type = ST_RECT;
					else if ( toks[0] == "cross" )
						type = ST_CROSS;
					else if ( toks[0] == "none" ) {
						shown = false;
						return;
					}
					else {
						SEISCOMP_WARNING("%s: wrong shape shape type: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
				}

				if ( toks.size() > 1 ) {
					if ( !Core::fromString(size, toks[1]) || size < 0 ) {
						SEISCOMP_WARNING("%s: invalid size: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
				}

				setSize(size);

				if ( toks.size() > 2 ) {
					QColor color;
					if ( toks[2] == "none" ) {
						fill = Qt::NoBrush;
						fillUsed = true;
					}
					else if ( !fromString(color, toks[2]) ) {
						SEISCOMP_WARNING("%s: wrong color definition: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
					else {
						fill = color;
						fillUsed = true;
					}
				}

				if ( toks.size() > 3 ) {
					QColor color;
					if ( toks[3] == "none" ) {
						stroke = Qt::NoPen;
					}
					else if ( !fromString(color, toks[3]) ) {
						SEISCOMP_WARNING("%s: wrong color definition: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
					else
						stroke = color;
				}
			}


			void draw(QPainter &p) const {
				if ( !shown ) return;
				p.setPen(stroke);
				drawWithoutStroke(p);
			}


			void drawWithoutStroke(QPainter &p) const {
				switch ( type ) {
					case ST_CIRCLE:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawEllipse(-size/2,-size/2,size,size);
						break;
					case ST_CROSS:
						if ( fillUsed )
							p.setPen(fill.color());
						p.drawLine(-size/2,-size/2,size/2,size/2);
						p.drawLine(-size/2,size/2,size/2,-size/2);
						break;
					case ST_RECT:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawRect(-size/2,-size/2,size,size);
						break;
					case ST_TRIANGLE:
					case ST_TRIANGLE2:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawPolygon(poly);
						break;
				};
			}


			ShapeType type;
			int       size;
			bool      fillUsed;
			QBrush    fill;
			QPen      stroke;
			bool      shown;
			QPolygonF poly;
		};


		enum StationNameMode {
			SNM_OFF,
			SNM_OUTWARDS,
			SNM_INWARDS
		};


	public:
		PlotWidget(QWidget *parent = 0, ArrivalModel *model = 0)
		: OriginLocatorPlot(parent), _model(model), _commitButton(this) {
			//_renderer.setTColor(QColor(224,224,224));
			//_renderer.setShadingEnabled(true);
			_dragStarted = false;
			_customDraw = false;
			_drawStationNames = SNM_OFF;
			_dirty = false;
			_preferredTensorDirty = false;
			_preferredTensorValid = false;
			_showPreferredFM = true;

			_npLabel = new QLabel(this);
			//_npLabel->setCursor(Qt::PointingHandCursor);
			_npLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
			_npLabel->setVisible(_customDraw);

			_commitButton.setVisible(_customDraw);
			_commitButton.setText("C");
			_commitButton.setToolTip("Confirm and send the current solution.");
			_commitButton.setMenu(new QMenu(this));
			QAction *commitWithMTAction = _commitButton.menu()->addAction("With MT solution...");

			connect(_npLabel, SIGNAL(linkActivated(const QString &)),
			        this, SLOT(linkClicked()));
			connect(&_commitButton, SIGNAL(clicked(bool)),
			        this, SLOT(commitButtonClicked(bool)));
			connect(commitWithMTAction, SIGNAL(triggered(bool)),
			        this, SLOT(commitWithMTTriggered(bool)));

			set(90,90,0);

			_renderer.setShadingEnabled(true);
			_renderer.setMaterial(0.8,0.2);

			// Negative
			_shapes[POL_POSITIVE] = Shape(ST_CIRCLE, 10);
			_shapes[POL_POSITIVE].init("olv.fmplot.shape.polarity.positive");
			_shapes[POL_NEGATIVE] = Shape(ST_CIRCLE, 10, Qt::NoBrush);
			_shapes[POL_NEGATIVE].init("olv.fmplot.shape.polarity.negative");
			_shapes[POL_UNDECIDABLE] = Shape(ST_CROSS, 10);
			_shapes[POL_UNDECIDABLE].init("olv.fmplot.shape.polarity.undecidable");
			_shapes[POL_UNSET] = Shape();
			_shapes[POL_UNSET].init("olv.fmplot.shape.polarity.unset");

			_shapeAxis[0] = Shape(ST_TRIANGLE, 14, Qt::red, QPen(Qt::black));
			_shapeAxis[0].init("olv.fmplot.shape.t-axis");
			_shapeAxis[1] = Shape(ST_TRIANGLE, 14, Qt::NoBrush, QPen(Qt::red));
			_shapeAxis[1].init("olv.fmplot.shape.p-axis");
		}


		const Shape &shape(PolarityType type) {
			return _shapes[type];
		}


		void resetPreferredFM() {
			_preferredFMBuffer = QImage();
			_preferredTensor = Math::Tensor2Sd();
			_preferredTensorValid = false;
			update();
		}


		void setPreferredFM(const Math::Tensor2Sd &t) {
			_preferredTensor = t;
			_preferredTensorValid = true;
			_preferredTensorDirty = true;
			update();
		}


		void setPreferredFM(double str, double dip, double rake) {
			NODAL_PLANE np;
			np.str = str;
			np.dip = dip;
			np.rake = rake;

			Math::np2tensor(np, _preferredTensor);
			setPreferredFM(_preferredTensor);
		}


		void setCustomDraw(bool f) {
			_dragStarted = false;
			_customDraw = f;
			_dirty = false;
			_preferredTensorDirty = false;
			_npLabel->setVisible(_customDraw);
			_commitButton.setVisible(_customDraw);
		}


		void set(double str, double dip, double rake) {
			_np1.str = str;
			_np1.dip = dip;
			_np1.rake = rake;

			Math::np2tensor(_np1, _tensor);
			set(_tensor);
		}

		void set(const Math::Tensor2Sd &tensor) {
			_tensor = tensor;
			Math::Spectral2Sd spec;
			Math::Vector3d tn, td;

			spec.spect(_tensor);
			spec.sort();

			Math::AXIS t,n,p;
			Math::spectral2axis(spec, t,n,p, 0);
			Math::pa2nd(spec.n1, spec.n3, tn, td);
			Math::nd2dc(tn, td, &_np1, &_np2);

			_tAxis = equalarea(t.str, t.dip);
			_pAxis = equalarea(p.str, p.dip);

			_npLabel->setText(QString("NP1: <a href=\"np1\">%1/%2/%3</a> "
			                          "NP2: <a href=\"np2\">%4/%5/%6</a>")
			                  .arg(_np1.str, 0, 'f', 0)
			                  .arg(_np1.dip, 0, 'f', 0)
			                  .arg(_np1.rake, 0, 'f', 0)
			                  .arg(_np2.str, 0, 'f', 0)
			                  .arg(_np2.dip, 0, 'f', 0)
			                  .arg(_np2.rake, 0, 'f', 0));

			Math::Matrix3f tmp;
			Math::tensor2matrix(_tensor, tmp);

			_renderer.setPColor(palette().color(QPalette::Base));
			_renderer.setTColor(palette().color(QPalette::Mid));
			_renderer.setBorderColor(palette().color(QPalette::WindowText));
			_renderer.render(_buffer, _tensor, tmp);

			update();
		}

		const NODAL_PLANE &np1() const {
			return _np1;
		}

		const NODAL_PLANE &np2() const {
			return _np2;
		}


	protected:
		void updateContextMenu(QMenu &menu) {
			if ( !_customDraw ) return;
			menu.addSeparator();
			QMenu *subShowStations = menu.addMenu("Draw station names");
			QAction *act1 = subShowStations->addAction("Outwards");
			act1->setCheckable(true);
			act1->setChecked(_drawStationNames == SNM_OUTWARDS);
			act1->setData(1001);
			QAction *act2 = subShowStations->addAction("Inwards");
			act2->setCheckable(true);
			act2->setChecked(_drawStationNames == SNM_INWARDS);
			act2->setData(1002);
			QAction *act3 = subShowStations->addAction("Off");
			act3->setCheckable(true);
			act3->setChecked(_drawStationNames == SNM_OFF);
			act3->setData(1000);

			QActionGroup *group = new QActionGroup(&menu);
			group->addAction(act1);
			group->addAction(act2);
			group->addAction(act3);

			QAction *act = menu.addAction("Enable shading");
			act->setCheckable(true);
			act->setChecked(_renderer.isShadingEnabled());

			act = menu.addAction("Show preferred solution (if available)");
			act->setCheckable(true);
			act->setChecked(_showPreferredFM);
		}

		void handleContextMenuAction(QAction *action) {
			OriginLocatorPlot::handleContextMenuAction(action);
			if ( action == nullptr ) return;
			if ( action->data().toInt() == 1000 ) {
				_drawStationNames = SNM_OFF;
				update();
			}
			else if ( action->data().toInt() == 1001 ) {
				_drawStationNames = SNM_OUTWARDS;
				update();
			}
			else if ( action->data().toInt() == 1002 ) {
				_drawStationNames = SNM_INWARDS;
				update();
			}
			else if ( action->text() == "Enable shading" ) {
				_renderer.setShadingEnabled(action->isChecked());
				_dirty = true;
				_preferredTensorDirty = true;
				update();
			}
			else if ( action->text() == "Show preferred solution (if available)" ) {
				_showPreferredFM = action->isChecked();
				update();
			}
		}

		void linkClicked() {
			NodalPlaneDialog dlg(this);
			dlg.sbStrike->setValue(_np1.str);
			dlg.sbDip->setValue(_np1.dip);
			dlg.sbRake->setValue(_np1.rake);
			if ( dlg.exec() != QDialog::Accepted ) return;
			set(dlg.sbStrike->value(), dlg.sbDip->value(),dlg.sbRake->value());
		}

		void commitButtonClicked(bool) {
			emit focalMechanismCommitted();
		}

		void commitWithMTTriggered(bool) {
			QMenu *m = _commitButton.menu();
			emit focalMechanismCommitted(true, m->mapToGlobal(QPoint()));
		}

		void mousePressEvent(QMouseEvent *event) {
			if ( !_customDraw ) {
				DiagramWidget::mousePressEvent(event);
				return;
			}

			if ( event->button() != Qt::LeftButton ) return;

			if ( hoveredValue() != -1 ) {
				emit clicked(hoveredValue());
				return;
			}

			if ( event->pos().y() < diagramRect().top() ) return;

			_startDragPos = event->pos();
			_dragStarted = true;
		}


		void mouseReleaseEvent(QMouseEvent *event) {
			DiagramWidget::mouseReleaseEvent(event);
			if ( event->button() != Qt::LeftButton ) return;
			_dragStarted = false;
		}


		void mouseMoveEvent(QMouseEvent *event) {
			if ( !_customDraw || !_dragStarted ) {
				DiagramWidget::mouseMoveEvent(event);
				return;
			}

			QPoint delta = event->pos() - _startDragPos;

			double deltaX = (double)delta.x() / (double)diagramRect().width();
			double deltaY = (double)delta.y() / (double)diagramRect().height();

			Math::Matrix3d m, mx, my;
			mx.loadRotateY(deg2rad(-deltaY * 180));
			my.loadRotateX(deg2rad(-deltaX * 180));
			m.mult(mx, my);

			_tensor.rotate(m);
			set(_tensor);

			_startDragPos = event->pos();

			update();
		}


		void diagramAreaUpdated(const QRect &rect) {
			_dirty = true;
			_preferredTensorDirty = true;
			_npLabel->setGeometry(0,0,width(),diagramRect().top());
			_commitButton.move(width()-_commitButton.width(),_npLabel->height()+4);
		}


		void paintSphericalBackground(QPainter &painter) {
			if ( !_customDraw ) {
				DiagramWidget::paintSphericalBackground(painter);
				return;
			}

			if ( _dirty ) {
				_buffer = QImage(diagramRect().size(), QImage::Format_ARGB32);
				Math::Matrix3f m;
				Math::tensor2matrix(_tensor, m);
				_renderer.setPColor(palette().color(QPalette::Base));
				_renderer.setTColor(palette().color(QPalette::Mid));
				_renderer.setBorderColor(palette().color(QPalette::WindowText));
				_renderer.render(_buffer, _tensor, m);
				_dirty = false;
			}

			if ( _preferredTensorDirty && _preferredTensorValid && _showPreferredFM ) {
				_preferredFMBuffer = QImage(diagramRect().size(), QImage::Format_ARGB32);
				Math::Matrix3f m;
				Math::tensor2matrix(_preferredTensor, m);
				_renderer.setPColor(palette().color(QPalette::Base));
				_renderer.setTColor(palette().color(QPalette::Highlight));
				_renderer.setBorderColor(palette().color(QPalette::WindowText));
				_renderer.render(_preferredFMBuffer, _preferredTensor, m);

				// Blend complete buffer 25%
				uchar *data = _preferredFMBuffer.bits();
				for ( int y = 0; y < _preferredFMBuffer.height(); ++y ) {
					QRgb *rgb = (QRgb*)data;
					for ( int x = 0; x < _preferredFMBuffer.width(); ++x, ++rgb )
						*rgb = (*rgb & 0x00FFFFFF) | (((*rgb >> 24)*1/4) << 24);
					data += _preferredFMBuffer.bytesPerLine();
				}
			}

			painter.drawImage(diagramRect().topLeft(), _buffer);

			if ( _showPreferredFM )
				painter.drawImage(diagramRect().topLeft(), _preferredFMBuffer);

			QRectF tmp(_displayRect);
			_displayRect.setRight(1);

			QPointF p;

			painter.setRenderHint(QPainter::Antialiasing, true);

			// Draw T Axis
			p = (this->*project)(_tAxis);
			painter.translate(p);
			_shapeAxis[0].draw(painter);
			painter.translate(-p);

			// Draw P Axis
			p = (this->*project)(_pAxis);
			painter.translate(p);
			_shapeAxis[1].draw(painter);
			painter.translate(-p);

			_displayRect = tmp;

			painter.setRenderHint(QPainter::Antialiasing, false);
		}


		void drawValue(int id, QPainter& painter, const QPointF &p,
		               SymbolType type, bool valid) const {
			if ( _customDraw ) {
				if ( valid ) {
					painter.setRenderHint(QPainter::Antialiasing);

					int pol = value(id, PC_POLARITY);

					painter.translate(p.x(), p.y());

					_shapes[pol].drawWithoutStroke(painter);

					if ( _drawStationNames != SNM_OFF && _model ) {
						double az = value(id, PC_FMAZI);
						QString sta =
							_model->data(_model->index(id, STATION), Qt::DisplayRole).toString();
						painter.setPen(Qt::black);
						if ( az >= 0 && az <= 180 ) {
							painter.rotate(az-90);
							if ( _drawStationNames == SNM_INWARDS )
								painter.drawText(-(_shapes[pol].size/2+2)-width(),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignRight, sta);
							else
								painter.drawText((_shapes[pol].size/2+2),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignLeft, sta);

							painter.rotate(90-az);
						}
						else {
							painter.rotate(az+90);
							if ( _drawStationNames == SNM_INWARDS )
								painter.drawText((_shapes[pol].size/2+2),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignLeft, sta);
							else
								painter.drawText(-(_shapes[pol].size/2+2)-width(),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignRight, sta);
							painter.rotate(-az-90);
						}
					}

					painter.translate(-p.x(), -p.y());
				}
			}
			else
				DiagramWidget::drawValue(id, painter, p, type, valid);
		}


	private:
		Shape             _shapes[POL_QUANTITY];
		Shape             _shapeAxis[2];
		ArrivalModel     *_model;
		QToolButton       _commitButton;
		QLabel           *_npLabel;
		QImage            _buffer;
		QImage            _preferredFMBuffer;
		QPointF           _tAxis, _pAxis;
		TensorRenderer    _renderer;
		bool              _customDraw;
		StationNameMode   _drawStationNames;
		bool              _dirty;
		Math::Tensor2Sd   _tensor;
		Math::NODAL_PLANE _np1, _np2;

		bool              _showPreferredFM;
		bool              _preferredTensorDirty;
		bool              _preferredTensorValid;
		Math::Tensor2Sd   _preferredTensor;

		QPoint            _startDragPos;
		bool              _dragStarted;
};


std::string wfid2str(const DataModel::WaveformStreamID &id) {
	return id.networkCode() + "." + id.stationCode() + "." +
	       id.locationCode();
}

QString wfid2qstr(const DataModel::WaveformStreamID &id) {
	return QString("%1.%2.%3.%4")
	       .arg(id.networkCode().c_str())
	       .arg(id.stationCode().c_str())
	       .arg(id.locationCode().c_str())
	       .arg(id.channelCode().c_str());
}


typedef std::pair<std::string, std::string> PickPhase;
typedef std::pair<PickPtr, int> PickWithFlags;
typedef std::map<PickPhase, PickWithFlags> PickedPhases;


}


ArrivalDelegate::ArrivalDelegate(QWidget *parent)
: QStyledItemDelegate(parent)
, _margin(2), _spacing(4), _statusRectWidth(6), _labelWidth(0) {
	_flags[0] = Seismology::LocatorInterface::F_TIME;
	_flags[1] = Seismology::LocatorInterface::F_SLOWNESS;
	_flags[2] = Seismology::LocatorInterface::F_BACKAZIMUTH;

	_labels[0] = "T";
	_labels[1] = "S";
	_labels[2] = "B";

	if ( parent )
		_statusRectWidth = QT_FM_WIDTH(parent->fontMetrics(), 'A');
}

bool ArrivalDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) {
	if ( index.column() != USED )
		return QStyledItemDelegate::editorEvent(event, model, option, index);

	if ( event->type() == QEvent::MouseButtonPress ||
	     event->type() == QEvent::MouseButtonDblClick ) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if ( mouseEvent->buttons() & Qt::LeftButton ) {
			QPoint pos = mouseEvent->pos();

			bool ret = QStyledItemDelegate::editorEvent(event, model, option, index);

			QList<QRect> rects;
			getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

			if ( rects[4].contains(pos) ) {
				int flags = (Qt::CheckState)index.data(UsedRole).toInt();
				if ( flags == 0 ) {
					model->setData(index, 1, RestoreRole);
				}
				else {
					model->setData(index, 0, RestoreRole);
				}
			}
			else {
				int flags = index.data(UsedRole).toInt(),
				    mask = getMask(index);
				for ( int i = 0; i < 3; ++i ) {
					bool enabled = mask & _flags[i];
					if ( !enabled ) continue;

					if ( rects[i].contains(pos) ) {
						if ( flags & _flags[i] )
							flags &= ~_flags[i];
						else
							flags |= _flags[i];

						model->setData(index, flags, UsedRole);
					}
				}
			}

			return ret;
		}
	}
	else if ( event->type() == QEvent::MouseMove ) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		QPoint pos = mouseEvent->pos();

		QList<QRect> rects;
		getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

		int hover = -1,
		    mask = getMask(index);
		for ( int i = 0; i < 3; ++i ) {
			bool enabled = mask & _flags[i];
			if ( !enabled ) continue;

			if ( rects[i].contains(pos) ) {
				hover = i;
				break;
			}
		}

		model->setData(index, hover, HoverRole);

		return false;
	}

	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool ArrivalDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) {
	if ( index.column() != USED )
		return QStyledItemDelegate::helpEvent(event, view, option, index);

	if ( event->type() == QEvent::ToolTip ) {
		QPoint pos = event->pos();

		QList<QRect> rects;
		getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

		if ( rects[4].contains(pos) ) {
			QToolTip::showText(event->globalPos(),
			                   tr("Toggle if arrival should be used or not."),
			                   view);
			return true;
		}
		else {
			static const char *FlagNames[3] = {"time", "slowness", "backazimuth"};

			int mask = getMask(index);

			for ( int i = 0; i < 3; ++i ) {
				if ( !rects[i].contains(pos) ) continue;

				bool enabled = mask & _flags[i];
				if ( !enabled ) {
					QToolTip::showText(event->globalPos(),
					                   tr("The pick does not have a %1 value and the usage flag is therefore disabled.")
					                   .arg(FlagNames[i]),
					                   view);
					return true;
				}

				QToolTip::showText(event->globalPos(),
				                   tr("Toggle %1 usage.")
				                   .arg(FlagNames[i]),
				                   view);
				return true;
			}
		}
	}

	return QStyledItemDelegate::helpEvent(event, view, option, index);
}

void ArrivalDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
	if ( index.column() != USED ) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	painter->save();

	QPen pen = painter->pen();
	if ( option.state & QStyle::State_Selected ) {
		painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
		pen.setColor(option.palette.color(QPalette::HighlightedText));
	}
	else {
		pen.setColor(option.palette.color(QPalette::WindowText));
	}

	QList<QRect> rects;
	getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

	QRect statusRect = rects[3];//.center().x() - 4, rects[3].center().y() - 4, 9, 9);

	painter->fillRect(statusRect, index.data(Qt::BackgroundRole).value<QColor>());

	QStyleOptionButton boxStyle;
	boxStyle.state = QStyle::State_Enabled;

	int flags = index.data(UsedRole).toInt(),
	    mask = getMask(index);

	flags &= mask;

	if( flags == mask ) {
		boxStyle.state |= QStyle::State_On;
	}
	else if ( flags ) {
		boxStyle.state |= QStyle::State_NoChange;
	}
	else {
		boxStyle.state |= QStyle::State_Off;
	}

	boxStyle.direction = QApplication::layoutDirection();
	boxStyle.rect = rects[4];
	QApplication::style()->drawControl(QStyle::CE_CheckBox, &boxStyle, painter);

	int hoverIndex = index.data(HoverRole).toInt();

	for ( int i = 0; i < 3; ++i ) {
		if ( i == hoverIndex && option.state & QStyle::State_MouseOver ) {
			QFont font = option.font;
			font.setBold(true);
			painter->setFont(font);
		}

		bool enabled = mask & _flags[i];
		bool checked = (flags & _flags[i]) && enabled;
		if ( !enabled ) {
			if ( option.state & QStyle::State_Selected )
				painter->setPen(option.palette.color(QPalette::Disabled, QPalette::HighlightedText));
			else
				painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
		}
		else
			painter->setPen(pen);

		painter->drawText(rects[i], Qt::AlignVCenter | Qt::AlignHCenter,
		                  checked ? _labels[i] : (enabled ? "n" : "-"));
		painter->setFont(option.font);
	}

	painter->restore();
}

QSize ArrivalDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
	if ( index.column() != USED ) {
		return QStyledItemDelegate::sizeHint(option, index);
	}

	QFont font = option.font;
	font.setBold(true);

	_labelWidth = 0;

	font.setPointSize(font.pointSize() + 2);

	QFontMetrics fm(font);
	int labelHeight = 0;
	for ( int i = 0; i < 3; ++i ) {
		QRect rect = fm.boundingRect(_labels[i]);
		_labelWidth = max(_labelWidth, rect.width());
		labelHeight = max(labelHeight, rect.height());
	}

	QStyle *style = qApp->style();
	int checkBoxWidth = style->subElementRect(QStyle::SE_CheckBoxIndicator,
	                                          &option).width();

	int width = 2 * _margin + _statusRectWidth + 3 * _labelWidth +
	            4 * _spacing + checkBoxWidth,
	    height = 2 * _margin + max(labelHeight,
	                               option.decorationSize.height());

	return QSize(width, height);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Implementation of ArrivalModel

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalModel::ArrivalModel(DataModel::Origin* origin, QObject *parent)
 : QAbstractTableModel(parent) {

	_disabledForeground = Qt::gray;

	for ( int i = 0; i < ArrivalListColumns::Quantity; ++i ) {
		switch ( i ) {
			case DISTANCE:
				if ( SCScheme.unit.distanceInKM )
					_header << QString("%1 (km)").arg(EArrivalListColumnsNames::name(i));
				else
					_header << QString("%1 (°)").arg(EArrivalListColumnsNames::name(i));
				break;
			case RESIDUAL:
			case UNCERTAINTY:
			case LATENCY:
				_header << QString("%1 (s)").arg(EArrivalListColumnsNames::name(i));
				break;
			case SLOWNESS:
			case SLOWNESS_RESIDUAL:
				_header << QString("%1 (s/°)").arg(EArrivalListColumnsNames::name(i));
				break;
			case AZIMUTH:
			case TAKEOFF:
			case BACKAZIMUTH:
			case BACKAZIMUTH_RESIDUAL:
				_header << QString("%1 (°)").arg(EArrivalListColumnsNames::name(i));
				break;
			case TIME:
				if ( SCScheme.dateTime.useLocalTime )
					_header << QString("%1 (%2)").arg(EArrivalListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
				else
					_header << QString("%1 (UTC)").arg(EArrivalListColumnsNames::name(i));
				break;
			default:
				_header << EArrivalListColumnsNames::name(i);
				break;
		}
	}

	setOrigin(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setOrigin(DataModel::Origin* origin) {
	_pickTimeFormat = "%T.%";
	_pickTimeFormat += Core::toString(SCScheme.precision.pickTime);
	_pickTimeFormat += "f";

	_origin = origin;
	if ( _origin ) {
		_used.fill(Seismology::LocatorInterface::F_NONE, _origin->arrivalCount());
		_backgroundColors.fill(QVariant(), _origin->arrivalCount());
		_enableState.fill(true, _origin->arrivalCount());
		_distances.fill(QVariant(), _origin->arrivalCount());
		_takeOffs.fill(QVariant(), _origin->arrivalCount());
		_hoverState.fill(-1, _origin->arrivalCount());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setRowColor(int row, const QColor& c) {
	if ( row >= rowCount() ) return;
	_backgroundColors[row] = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ArrivalModel::rowCount(const QModelIndex &) const {
	return _origin?_origin->arrivalCount():0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ArrivalModel::columnCount(const QModelIndex &) const {
	return _header.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant ArrivalModel::data(const QModelIndex &index, int role) const {
	if ( !index.isValid() )
		return QVariant();

	if ( index.row() >= (int)_origin->arrivalCount() )
		return QVariant();

	if ( index.column() == USED ) {
		if ( role == UsedRole ) {
			return _used[index.row()] & F_DISABLED ? 0 : _used[index.row()];
		}
		else if ( role == HoverRole ) {
			return _hoverState[index.row()];
		}
	}

	Arrival *a = _origin->arrival(index.row());
	Pick *pick;
	char buf[10];

	if ( role == Qt::DisplayRole ) {
		switch ( index.column() ) {
			case USED:
				/*
				try {
					snprintf(buf, 10, "   %.3f", a->weight());
					return buf;
				}
				catch ( Core::ValueException& ) {}
				break;
				*/
				return QVariant();

			case CREATED:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						return timeToString(pick->creationInfo().creationTime(), "%T.%1f");
					}
					catch ( ValueException& ) {}
				}
				break;

			case STATUS:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						const char *strStat = pick->evaluationMode().toString();

						if ( pick->methodID().empty() )
							return QString("%1").arg(strStat && *strStat?(char)toupper(*strStat):'-');
						else
							return QString("%1<%2>").arg(strStat && *strStat?(char)toupper(*strStat):'-')
							                        .arg((char)toupper(pick->methodID()[0]));
					}
					catch ( ValueException& ) {}
				}
				break;

			case LATENCY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						Core::TimeSpan lcy = pick->creationInfo().creationTime() - pick->time();
						long lcy_secs = lcy.seconds();
						return QString("%1:%2:%3")
						         .arg(lcy_secs/3600, 2, 10, QChar('0'))
						         .arg((lcy_secs%3600)/60, 2, 10, QChar('0'))
						         .arg(lcy_secs % 60, 2, 10, QChar('0'));
					}
					catch ( ValueException& ) {}
				}
				break;

			// Phase
			case PHASE:
				return a->phase().code().c_str();
				break;

			case WEIGHT:
				try {
					snprintf(buf, 10, "%.2f", a->weight());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			// Networkcode
			case NETWORK:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return pick->waveformID().networkCode().c_str();
				break;

			// Stationcode
			case STATION:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return pick->waveformID().stationCode().c_str();
				break;

			// Locationcode + Channelcode
			case CHANNEL:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					if ( pick->waveformID().locationCode().empty() )
						return pick->waveformID().channelCode().c_str();
					else
						return (pick->waveformID().locationCode() + '.' + pick->waveformID().channelCode()).c_str();
				}
				break;

			// Picktime
			case TIME:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return timeToString(pick->time().value(), _pickTimeFormat.c_str());
				break;

			case UNCERTAINTY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						if ( pick->time().lowerUncertainty() == pick->time().upperUncertainty() )
							return QString("%1s").arg(pick->time().lowerUncertainty());
						else
							return QString("-%1s/+%2s")
							       .arg(pick->time().lowerUncertainty())
							       .arg(pick->time().upperUncertainty());
					}
					catch ( ... ) {}

					try {
						return QString("%1s").arg(pick->time().uncertainty());
					}
					catch ( ... ) {}
				}
				break;

			// Residual
			case RESIDUAL:
				try {
					snprintf(buf, 10, "%.2f", a->timeResidual());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			case BACKAZIMUTH:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				try {
					if ( pick )
						return pick->backazimuth().value();
				}
				catch ( ValueException& ) {}
				break;

			case BACKAZIMUTH_RESIDUAL:
				try {
					snprintf(buf, 10, "%.1f", a->backazimuthResidual());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			case SLOWNESS:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				try {
					if ( pick )
						return pick->horizontalSlowness().value();
				}
				catch ( ValueException& ) {}
				break;

			case SLOWNESS_RESIDUAL:
				try {
					snprintf(buf, 10, "%.2f", a->horizontalSlownessResidual());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			// Distance
			case DISTANCE:
				if ( _distances[index.row()].canConvert(QVariant::Double) ) {
					auto distance = _distances[index.row()].toDouble();
					if ( SCScheme.unit.distanceInKM ) {
						snprintf(buf, 10, "%.*f", SCScheme.precision.distance, distance);
					}
					else {
						snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
					}
					return buf;
				}
				return _distances[index.row()];

			// Azimuth
			case AZIMUTH:
				try {
					return (int)a->azimuth();
				}
				catch ( ValueException& ) {}
				break;

			case METHOD:
				pick = Pick::Find(a->pickID());
				if ( pick )
					return pick->methodID().c_str();
				break;

			case POLARITY:
				pick = Pick::Find(a->pickID());
				if ( pick ) {
					try {
						return pick->polarity().toString();
					}
					catch ( ... ) {}
				}
				break;

			case ONSET:
				pick = Pick::Find(a->pickID());
				if ( pick ) {
					try {
						return pick->onset().toString();
					}
					catch ( ... ) {}
				}
				break;

			case TAKEOFF:
				try {
					snprintf(buf, 10, "%.1f", a->takeOffAngle());
					return buf;
				}
				catch ( ... ) {
					if ( _takeOffs[index.row()].canConvert(QVariant::Double) ) {
						snprintf(buf, 10, "%.1f", _takeOffs[index.row()].toDouble());
						return buf;
					}
					return _takeOffs[index.row()];
				}
				break;

			case PUBLICID:
				pick = Pick::Find(a->pickID());
				if ( pick ) {
					return pick->publicID().c_str();
				}
				break;

			default:
				break;
		}
	}
	else if ( role == Qt::UserRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try { return fabs(a->timeResidual()); } catch ( ValueException& ) {}
				break;
			// Distance
			case DISTANCE:
				try { return a->distance(); } catch ( ValueException& ) {}
				break;
			case AZIMUTH:
				try { return a->azimuth(); } catch ( ValueException& ) {}
				break;
			case TAKEOFF:
				try {
					return a->takeOffAngle();
				}
				catch ( ValueException& ) {
					return _takeOffs[index.row()];
				}
				break;
			case WEIGHT:
				try { return a->weight(); } catch ( ValueException& ) {}
				break;
			case LATENCY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						Core::TimeSpan lcy = pick->creationInfo().creationTime() - pick->time();
						return (double)lcy;
					}
					catch ( ValueException& ) {}
				}
				break;
			default:
				break;
		}
	}
	else if ( role == Qt::BackgroundRole ) {
		switch ( index.column() ) {
			case USED:
				if ( index.row() < _backgroundColors.size() )
					return _backgroundColors[index.row()];
			default:
				return QVariant();
		}
	}
	else if ( role == Qt::ForegroundRole ) {
		if ( index.row() < _enableState.size() )
			return _enableState[index.row()]?QVariant():_disabledForeground;
	}
	/*
	else if ( role == Qt::TextColorRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try {
					return a->residual() < 0?Qt::darkRed:Qt::darkGreen;
				}
				catch ( ValueException& ) {}
				break;
			default:
				break;
		}
	}
	*/
	else if ( role == Qt::TextAlignmentRole ) {
		return colAligns[index.column()];
	}
	else if ( role == Qt::ToolTipRole ) {
		QString summary;
		int l = 0;
		pick = Pick::Cast(PublicObject::Find(a->pickID()));

		if ( pick ) {
			if ( l++ ) summary += '\n';
			summary += pick->publicID().c_str();
			if ( l++ ) summary += '\n';
			summary += wfid2qstr(pick->waveformID());
			if ( l++ ) summary += '\n';
			summary += timeToString(pick->time().value(), _pickTimeFormat.c_str());
		}

		/*
		// Phase
		if (l++) summary += '\n';
		summary += "Phase: ";
		try { summary += a->phase().code().c_str(); }
		catch ( ... ) { summary += "-"; }

		// Distance
		if (l++) summary += '\n';
		summary += "Distance: ";
		try {
			double distance = a->distance();
			if ( SCScheme.unit.distanceInKM )
				snprintf(buf, 10, "%.*f", SCScheme.precision.distance, Math::Geo::deg2km(distance));
			else
				snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
			summary += buf;
		}
		catch ( ... ) {
			summary += '-';
		}

		if ( SCScheme.unit.distanceInKM )
			summary += "km";
		else
			summary += degrees;

		// Azimuth
		if (l++) summary += '\n';
		summary += "Azimuth: ";
		try { summary += QString("%1").arg((int)a->azimuth()); }
		catch ( ... ) { summary += '-'; }
		summary += degrees;

		// Takeoff
		if (l++) summary += '\n';
		summary += "Takeoff: ";
		try { summary += QString("%1").arg((int)a->takeOffAngle()); }
		catch ( ... ) { summary += '-'; }
		summary += degrees;

		// Residual
		if (l++) summary += '\n';
		summary += "Residual: ";
		try { summary += QString("%1").arg(a->timeResidual(), 0, 'f', 2); }
		catch ( ... ) { summary += '-'; }
		summary += 's';
		*/

		// Filter
		if ( pick ) {
			if ( !pick->filterID().empty() ) {
				if (l++) summary += '\n';
				summary += "Filter: ";
				summary += pick->filterID().c_str();
			}

			if ( !pick->methodID().empty() ) {
				if (l++) summary += '\n';
				summary += "Method: ";
				summary += pick->methodID().c_str();
			}

			try {
				const CreationInfo &ci = pick->creationInfo();
				if ( !ci.author().empty() ) {
					if (l++) summary += '\n';
					summary += "Author: ";
					summary += ci.author().c_str();
				}
				if ( !ci.agencyID().empty() ) {
					if (l++) summary += '\n';
					summary += "Agency: ";
					summary += ci.agencyID().c_str();
				}
			}
			catch ( ... ) {}
		}

		return summary;
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant ArrivalModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
	if ( orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::TextAlignmentRole:
				return colAligns[section];
				break;
			case Qt::DisplayRole:
				if ( section >= _header.count() )
					return QString("%1").arg(section);
				else
					return _header[section];
				break;
		}
	}
	else {
		return section;
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::ItemFlags ArrivalModel::flags(const QModelIndex &index) const {
	if ( !index.isValid() )
		return Qt::ItemIsEnabled;

	return QAbstractTableModel::flags(index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::setData(const QModelIndex &index, const QVariant &value,
                           int role) {
	if ( index.isValid() && index.column() == USED ) {
		if ( !_enableState[index.row()] ) return false;

		if ( role == RestoreRole ) {
			if ( !value.toInt() ) {
				_used[index.row()] |= F_DISABLED;
			}
			else {
				_used[index.row()] &= ~F_DISABLED;
				if ( !_used[index.row()] ) {
					_used[index.row()] = getMask(index);
				}
			}
		}
		else if ( role == UsedRole ) {
			int flags = value.toInt() & getMask(index);
			if ( flags ) {
				_used[index.row()] = flags;
			}
			else {
				_used[index.row()] |= F_DISABLED;
			}
		}
		else if (role == HoverRole ){
			_hoverState[index.row()] = value.toInt();
		}
		else
			return QAbstractTableModel::setData(index, value, role);

		emit dataChanged(index, index);
		return true;
	}

	return QAbstractTableModel::setData(index, value, role);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setRowEnabled(int row, bool enabled) {
	if ( row >= _enableState.size() ) return;
	_enableState[row] = enabled;
	emit dataChanged(index(row,0), index(row,columnCount()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::isRowEnabled(int row) const {
	return _enableState[row];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setDistance(int row, const QVariant &val) {
	if ( row >= _distances.size() ) return;
	_distances[row] = val;
	emit dataChanged(index(row,DISTANCE), index(row,DISTANCE));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setTakeOffAngle(int row, const QVariant &val) {
	if ( row >= _takeOffs.size() ) return;
	_takeOffs[row] = val;
	emit dataChanged(index(row,TAKEOFF), index(row,TAKEOFF));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::useNoArrivals() const {
	for ( int i = 0; i < _used.count(); ++i ) {
		if ( useArrival(i) ) {
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::useArrival(int row) const {
	return ((_used[row] & F_DISABLED) == 0) &&
	       (_used[row] != Seismology::LocatorInterface::F_NONE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setUseArrival(int row, DataModel::Arrival *arrival) {
	try {
		setBackazimuthUsed(row, arrival->backazimuthUsed());
	}
	catch ( ... ) {
		Pick *pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		try {
			if ( pick ) {
				pick->backazimuth().value();
				setBackazimuthUsed(row, true);
			}
			else
				setBackazimuthUsed(row, false);
		}
		catch ( ValueException& ) {
			setBackazimuthUsed(row, false);
		}
	}

	try {
		setHorizontalSlownessUsed(row, arrival->horizontalSlownessUsed());
	}
	catch ( ... ) {
		Pick *pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		try {
			if ( pick ) {
				pick->horizontalSlowness().value();
				setHorizontalSlownessUsed(row, true);
			}
			else
				setHorizontalSlownessUsed(row, false);
		}
		catch ( ValueException& ) {
			setHorizontalSlownessUsed(row, false);
		}
	}

	try {
		setTimeUsed(row, arrival->timeUsed());
		double weight = 1.0;
		try {
			weight = fabs(arrival->weight());
		}
		catch ( ... ) {}

		if ( weight < 1E-6 ) {
			_used[row] |= F_DISABLED;
		}
	}
	catch ( ... ) {
		// If the timeUsed attribute is not set then it looks like an origin
		// created with an older version. So use the weight value to decide
		// whether the pick is active or not.
		double weight = 0.0;
		try {
			weight = fabs(arrival->weight());
		}
		catch ( ... ) {}

		if ( weight < 1E-6 ) {
			setBackazimuthUsed(row, false);
			setHorizontalSlownessUsed(row, false);
			setTimeUsed(row, false);
		}
		else {
			setTimeUsed(row, true);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::backazimuthUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;

	return (_used[row] & (Seismology::LocatorInterface::F_BACKAZIMUTH | F_DISABLED)) == Seismology::LocatorInterface::F_BACKAZIMUTH;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setBackazimuthUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled) {
		_used[row] |= Seismology::LocatorInterface::F_BACKAZIMUTH;
	}
	else {
		_used[row] &= ~Seismology::LocatorInterface::F_BACKAZIMUTH;
	}

	// Remove disabled state
	_used[row] &= ~F_DISABLED;

	emit dataChanged(index(row, USED),
	                 index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::horizontalSlownessUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;

	return (_used[row] & (Seismology::LocatorInterface::F_SLOWNESS | F_DISABLED)) == Seismology::LocatorInterface::F_SLOWNESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setHorizontalSlownessUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled) {
		_used[row] |= Seismology::LocatorInterface::F_SLOWNESS;
	}
	else {
		_used[row] &= ~Seismology::LocatorInterface::F_SLOWNESS;
	}

	// Remove disabled state
	_used[row] &= ~F_DISABLED;

	emit dataChanged(index(row, USED),
	                 index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::timeUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;

	return (_used[row] & (Seismology::LocatorInterface::F_TIME | F_DISABLED)) == Seismology::LocatorInterface::F_TIME;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setTimeUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled) {
		_used[row] |= Seismology::LocatorInterface::F_TIME;
	}
	else {
		_used[row] &= ~Seismology::LocatorInterface::F_TIME;
	}

	// Remove disabled state
	_used[row] &= ~F_DISABLED;

	emit dataChanged(index(row, USED), index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// Implementation of OriginLocatorView

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DiagramFilterSettingsDialog::DiagramFilterSettingsDialog(QWidget *parent)
: _ui(new Ui::FilterSettings) {
	_ui->setupUi(this);

	filterChanged(_ui->comboFilter->currentIndex());

	connect(_ui->comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(filterChanged(int)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DiagramFilterSettingsDialog::~DiagramFilterSettingsDialog() {
	delete _ui;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramFilterSettingsDialog::filterChanged(int idx) {
	_ui->frameNoFilter->setVisible(false);
	_ui->frameAzimuthAroundEpicenter->setVisible(false);

	switch ( idx ) {
		case 0:
			_ui->frameNoFilter->setVisible(true);
			break;
		case 1:
			_ui->frameAzimuthAroundEpicenter->setVisible(true);
			break;
	};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct AzimuthFilter : DiagramFilterSettingsDialog::Filter {
	AzimuthFilter(double c, double e) : start(c-e), len(2*e) {
		while ( start < 0 ) start += 360;
		while ( start >= 360 ) start -= 360;
	}

	bool accepts(DiagramWidget *w, int id) {
		float v = w->value(id, PC_AZIMUTH);
		float l = v - start;
		while ( l < 0 ) l += 360;
		while ( l >= 360 ) l -= 360;
		return l <= len;
	}

	double start;
	double len;
};

}

DiagramFilterSettingsDialog::Filter *DiagramFilterSettingsDialog::createFilter() const {
	switch ( _ui->comboFilter->currentIndex() ) {
		default:
		case 0:
			break;
		case 1:
			return new AzimuthFilter(_ui->spinAzimuthCenter->value(), _ui->spinAzimuthExtent->value());
	};

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView::Config::Config() {
	reductionVelocityP = 6.0;
	drawMapLines = true;
	drawGridLines = true;
	computeMissingTakeOffAngles = false;
	defaultEventRadius = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView(const MapsDesc &maps,
                                     const PickerView::Config &pickerConfig,
                                     QWidget * parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _d_ptr(new OriginLocatorViewPrivate(pickerConfig)) {
	SC_D.maptree = new Map::ImageTree(maps);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView(Map::ImageTree* mapTree,
                                     const PickerView::Config &pickerConfig,
                                     QWidget * parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _d_ptr(new OriginLocatorViewPrivate(pickerConfig)) {
	SC_D.maptree = mapTree;
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorPlot::OriginLocatorPlot(QWidget *parent) : DiagramWidget(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorPlot::linkClicked() {}
void OriginLocatorPlot::commitButtonClicked(bool) {}
void OriginLocatorPlot::commitWithMTTriggered(bool) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::init() {
	SC_D.ui.setupUi(this);

	SC_D.blinkWidget = nullptr;
	SC_D.newOriginStatus = CONFIRMED;

	QObject *drawFilter = new ElideFadeDrawer(this);

	SC_D.ui.labelRegion->setFont(SCScheme.fonts.heading3);
	SC_D.ui.labelRegion->installEventFilter(drawFilter);
	SC_D.ui.label_15->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_12->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_10->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_11->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_8->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_13->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_7->setFont(SCScheme.fonts.normal);
	SC_D.ui.label_9->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelTime->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelDepth->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelDepthUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelDepthError->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelDepthErrorUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLatitude->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelLatitudeUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLatitudeError->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLatitudeErrorUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLongitude->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelLongitudeUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLongitudeError->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelLongitudeErrorUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelNumPhases->setFont(SCScheme.fonts.highlight);
	SC_D.ui.labelNumPhasesUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelNumPhasesError->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelStdError->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelStdErrorUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.lbComment->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelComment->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelAzimuthGap->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelAzimuthGapUnit->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelMinDist->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelMinDistUnit->setFont(SCScheme.fonts.normal);

	ElideFadeDrawer *elider = new ElideFadeDrawer(this);

	/*
	SC_D.ui.lbEventID->setFont(SCScheme.fonts.normal);
	SC_D.ui.lbAgencyID->setFont(SCScheme.fonts.normal);
	SC_D.ui.lbUser->setFont(SCScheme.fonts.normal);
	SC_D.ui.lbEvaluation->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelEventID->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelAgency->setFont(SCScheme.fonts.highlight);
	//setBold(SC_D.ui.labelAgency, true);
	SC_D.ui.labelEvaluation->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelUser->setFont(SCScheme.fonts.highlight);
	//setBold(SC_D.ui.labelUser, true);

	SC_D.ui.lbMethod->setFont(SCScheme.fonts.normal);
	SC_D.ui.lbEarthModel->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelMethod->setFont(SCScheme.fonts.normal);
	SC_D.ui.labelEarthModel->setFont(SCScheme.fonts.normal);
	*/

	setBold(SC_D.ui.labelAgency, true);
	setBold(SC_D.ui.labelUser, true);

	SC_D.ui.labelEventID->installEventFilter(elider);
	SC_D.ui.labelAgency->installEventFilter(elider);
	SC_D.ui.labelUser->installEventFilter(elider);
	SC_D.ui.labelEvaluation->installEventFilter(elider);

	SC_D.ui.labelMethod->installEventFilter(elider);
	SC_D.ui.labelEarthModel->installEventFilter(elider);

	if ( SCScheme.unit.distanceInKM )
		SC_D.ui.labelMinDistUnit->setText("km");

	SC_D.ui.btnMagnitudes->setEnabled(false);
	SC_D.ui.btnMagnitudes->setVisible(false);

	SC_D.reader = nullptr;
	SC_D.plotFilter = nullptr;
	SC_D.plotFilterSettings = nullptr;

	SC_D.ui.btnCustom0->setVisible(false);
	SC_D.ui.btnCustom1->setVisible(false);

	SC_D.commitMenu = new QMenu(this);
	SC_D.actionCommitOptions = SC_D.commitMenu->addAction("With additional options...");

	SC_D.ui.btnCommit->setMenu(SC_D.commitMenu);

	SC_D.ui.editFixedDepth->setValidator(new QDoubleValidator(0, 1000.0, 3, SC_D.ui.editFixedDepth));
	SC_D.ui.editDistanceCutOff->setValidator(new QDoubleValidator(0, 25000.0, 3, SC_D.ui.editFixedDepth));
	SC_D.ui.editDistanceCutOff->setText("1000");

	SC_D.modelArrivalsProxy = nullptr;
	SC_D.modelArrivals.setDisabledForeground(palette().color(QPalette::Disabled, QPalette::Text));

	ArrivalDelegate *delegate = new ArrivalDelegate(SC_D.ui.tableArrivals);
	SC_D.ui.tableArrivals->horizontalHeader()->setSectionsMovable(true);
	SC_D.ui.tableArrivals->setItemDelegate(delegate);
	SC_D.ui.tableArrivals->setMouseTracking(true);
	SC_D.ui.tableArrivals->resizeColumnToContents(0);

	connect(SC_D.ui.tableArrivals->horizontalHeader(), SIGNAL(sectionClicked(int)),
	        SC_D.ui.tableArrivals, SLOT(sortByColumn(int)));

	connect(SC_D.ui.tableArrivals, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableArrivalsContextMenuRequested(const QPoint &)));

	connect(&SC_D.modelArrivals, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	connect(SC_D.ui.tableArrivals->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableArrivalsHeaderContextMenuRequested(const QPoint &)));

	SC_D.ui.tableArrivals->setContextMenuPolicy(Qt::CustomContextMenu);
	SC_D.ui.tableArrivals->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	SC_D.ui.tableArrivals->horizontalHeader()->setSortIndicatorShown(true);
	SC_D.ui.tableArrivals->horizontalHeader()->setSortIndicator(DISTANCE, Qt::AscendingOrder);
	//SC_D.ui.tableArrivals->horizontalHeader()->setStretchLastSection(true);

	SC_D.ui.tableArrivals->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	SC_D.ui.tableArrivals->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QAction *action = new QAction(this);
	action->setShortcut(Qt::Key_Escape);
	connect(action, SIGNAL(triggered()),
	        SC_D.ui.tableArrivals, SLOT(clearSelection()));

	addAction(action);

	SC_D.residuals = new PlotWidget(SC_D.ui.groupResiduals, &SC_D.modelArrivals);
	SC_D.residuals->setEnabled(false);
	SC_D.residuals->setColumnCount(PlotCols::Quantity);

	try {
		SC_D.residuals->setDrawErrorBars(SCApp->configGetBool("olv.arrivalPlot.showUncertainties"));
	}
	catch ( ... ) {}

	SC_D.residuals->setErrorBarPens(SCScheme.colors.arrivals.uncertainties, SCScheme.colors.arrivals.defaultUncertainties);
	SC_D.residuals->setValueDisabledColor(SCScheme.colors.arrivals.disabled);
	SC_D.residuals->setDisplayRect(QRectF(0,-10,180,20));

	SC_D.map = new OriginLocatorMap(SC_D.maptree.get(), SC_D.ui.frameMap);
	SC_D.map->setMouseTracking(true);
	SC_D.map->setOriginCreationEnabled(true);

	try {
		SC_D.map->setStationsMaxDist(SCApp->configGetDouble("olv.map.stations.unassociatedMaxDist"));
	}
	catch ( ... ) {
		SC_D.map->setStationsMaxDist(360);
	}

	// Read custom column configuration
	try {
		std::vector<std::string> processProfiles =
			SCApp->configGetStrings("display.origin.addons");

		QGridLayout *grid = static_cast<QGridLayout*>(SC_D.ui.groupBox->layout());
		int row, col, rowSpan, colSpan;

		if ( !processProfiles.empty() ) {
			grid->getItemPosition(grid->indexOf(SC_D.ui.frameInfoSeparator),
			                      &row, &col, &rowSpan, &colSpan);
		}

		for ( size_t i = 0; i < processProfiles.size(); ++i ) {
			QString label, script;
			try {
				label = SCApp->configGetString("display.origin.addon." + processProfiles[i] + ".label").c_str();
			}
			catch ( ... ) { label = ""; }

			try {
				script = Environment::Instance()->absolutePath(SCApp->configGetString("display.origin.addon." + processProfiles[i] + ".script")).c_str();
			}
			catch ( ... ) {}

			if ( script.isEmpty() ) {
				std::cerr << "WARNING: display.origin.addon."
				          << processProfiles[i] << ".script is not set: ignoring"
				          << std::endl;
				continue;
			}

			QLabel *addonLabel = new QLabel;
			addonLabel->setText(label + ":");
			addonLabel->setAlignment(SC_D.ui.lbEventID->alignment());
			QLabel *addonText = new QLabel;

			row = grid->rowCount();
			grid->addWidget(addonLabel, row, 0, 1, 1);
			grid->addWidget(addonText, row, 1, 1, grid->columnCount()-1);
			++row;

			SC_D.scriptLabelMap[script] = OriginLocatorViewPrivate::ScriptLabel(addonLabel, addonText);
		}
	}
	catch ( ... ) {}

	if ( !SC_D.scriptLabelMap.isEmpty() ) {
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultAvailable(const QString &, const QString &, const QString &, const QString &)),
		        this, SLOT(evalResultAvailable(const QString &, const QString &, const QString &, const QString &)));
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultError(const QString &, const QString &, const QString &, int)),
		        this, SLOT(evalResultError(const QString &, const QString &, const QString &, int)));
		connect(this, SIGNAL(newOriginSet(Seiscomp::DataModel::Origin *,
		                                  Seiscomp::DataModel::Event *,
		                                  bool, bool)),
		        this, SLOT(evaluateOrigin(Seiscomp::DataModel::Origin *,
		                                  Seiscomp::DataModel::Event *,
		                                  bool, bool)));
	}

	QHBoxLayout* hboxLayout = new QHBoxLayout(SC_D.ui.frameMap);
	hboxLayout->setObjectName("hboxLayoutMap");
	hboxLayout->setSpacing(6);
	hboxLayout->setMargin(0);
	hboxLayout->addWidget(SC_D.map);

	SC_D.plotTab = new QTabBar(SC_D.ui.groupResiduals);
	SC_D.plotTab->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	SC_D.plotTab->setShape(QTabBar::RoundedNorth);

	for ( int i = 0; i < PlotTabs::Quantity; ++i ) {
		SC_D.plotTab->addTab(EPlotTabsNames::name(i));
	}

	SC_D.plotTab->setCurrentIndex(0);

	QLayoutItem *item = SC_D.ui.groupResiduals->layout()->takeAt(0);

	SC_D.ui.groupResiduals->layout()->addWidget(SC_D.plotTab);
	SC_D.ui.groupResiduals->layout()->addItem(item);
	SC_D.ui.groupResiduals->layout()->addWidget(SC_D.residuals);

	connect(SC_D.plotTab, SIGNAL(currentChanged(int)), this, SLOT(plotTabChanged(int)));

	plotTabChanged(SC_D.plotTab->currentIndex());
	connect(SC_D.ui.labelPlotFilter, SIGNAL(linkActivated(const QString &)), this, SLOT(changePlotFilter()));

	connect(SC_D.residuals, SIGNAL(valueActiveStateChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(SC_D.residuals, SIGNAL(endSelection()), this, SLOT(residualsSelected()));
	connect(SC_D.residuals, SIGNAL(adjustZoomRect(QRectF&)), this, SLOT(adjustResidualsRect(QRectF&)));
	connect(SC_D.residuals, SIGNAL(hover(int)), this, SLOT(hoverArrival(int)));
	connect(SC_D.residuals, SIGNAL(clicked(int)), this, SLOT(selectArrival(int)));
	connect(SC_D.residuals, SIGNAL(focalMechanismCommitted(bool, QPoint)),
	        this, SLOT(commitFocalMechanism(bool)));

	connect(SC_D.map, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(SC_D.map, SIGNAL(hoverArrival(int)), this, SLOT(hoverArrival(int)));
	connect(SC_D.map, SIGNAL(clickedArrival(int)), this, SLOT(selectArrival(int)));
	connect(SC_D.map, SIGNAL(clickedStation(const std::string &, const std::string &)),
	        this, SLOT(selectStation(const std::string &, const std::string &)));
	connect(SC_D.map, SIGNAL(artificialOriginRequested(const QPointF &, const QPoint &)),
	        this, SLOT(createArtificialOrigin(const QPointF &, const QPoint &)));

	//connect(SC_D.ui.btnZoom, SIGNAL(clicked()), this, SLOT(zoomMap()));

	connect(SC_D.ui.btnImportAllArrivals, SIGNAL(clicked()), this, SLOT(importArrivals()));
	connect(SC_D.ui.btnShowWaveforms, SIGNAL(clicked()), this, SLOT(showWaveforms()));
	//connect(SC_D.ui.btnShowWaveforms, SIGNAL(clicked()), this, SIGNAL(waveformsRequested()));
	connect(SC_D.ui.btnRelocate, SIGNAL(clicked()), this, SLOT(relocate()));
	connect(SC_D.ui.btnMagnitudes, SIGNAL(clicked()), this, SLOT(computeMagnitudes()));
	connect(SC_D.ui.buttonEditComment, SIGNAL(clicked()), this, SLOT(editComment()));
	connect(SC_D.ui.btnCommit, SIGNAL(clicked()), this, SLOT(commit()));
	connect(SC_D.actionCommitOptions, SIGNAL(triggered()), this, SLOT(commitWithOptions()));

	connect(&SC_D.blinkTimer, SIGNAL(timeout()), this, SLOT(updateBlinkState()));

	/*
	QFontMetrics fm = fontMetrics();
	int width = SC_D.ui.lbAgencyID->width() + 6 + fm.boundingRect("WWWWWWWWWW").width();

	SC_D.ui.groupBox->setFixedWidth(width);
	*/

	SC_D.displayComment = false;
	try {
		SC_D.displayCommentID = SCApp->configGetString("olv.display.origin.comment.id");
		SC_D.displayComment = true;
	}
	catch ( ... ) {}

	SC_D.ui.lbComment->setVisible(SC_D.displayComment);
	SC_D.ui.labelComment->setVisible(SC_D.displayComment);

	try {
		SC_D.displayCommentDefault = SCApp->configGetString("display.origin.comment.default");
	}
	catch ( ... ) {
		SC_D.displayCommentDefault = SC_D.ui.labelComment->text().toStdString();
	}

	try {
		SC_D.ui.lbComment->setText(QString("%1:").arg(SCApp->configGetString("display.origin.comment.label").c_str()));
	}
	catch ( ... ) {
		SC_D.ui.lbComment->setText(SC_D.displayCommentID.c_str());
	}

	try {
		EventType et;
		if ( et.fromString(SCApp->configGetString("olv.defaultEventType").c_str()) )
			SC_D.defaultEventType = et;
		else
			cerr << "ERROR: unknown type '" << SCApp->configGetString("olv.defaultEventType")
			     << "' in olv.defaultEventType" << endl;
	}
	catch ( ... ) {}

	try {
		SC_D.defaultEarthModel = SCApp->configGetString("olv.locator.defaultProfile");
	}
	catch ( ... ) {}

	std::string defaultLocator = "LOCSAT";
	try {
		defaultLocator = SCApp->configGetString("olv.locator.interface");
	}
	catch ( ... ) {
		try {
			defaultLocator = SCApp->configGetString("olv.locator");
		}
		catch ( ... ) {}
	}

	{
		QVector<bool> usedFlags(DataModel::EOriginDepthTypeQuantity, false);
		QColor reducedColor;
		reducedColor = blend(palette().color(QPalette::Text), palette().color(QPalette::Base), 75);

		SC_D.ui.cbDepthType->addItem("depth type set by locator");
		SC_D.ui.cbDepthType->addItem("- unset -");

		try {
			vector<string> depthTypes = SCApp->configGetStrings("olv.commonDepthTypes");
			for ( size_t i = 0; i < depthTypes.size(); ++i ) {
				DataModel::OriginDepthType type;
				if ( !type.fromString(depthTypes[i].c_str()) ) {
					SEISCOMP_WARNING("olv.commonDepthTypes: invalid type, ignoring: %s",
					                 depthTypes[i].c_str());
				}
				else {
					usedFlags[type.toInt()] = true;
					SC_D.ui.cbDepthType->addItem(type.toString(), type.toInt());
				}
			}
		}
		catch ( ... ) {}

		for ( int i = 0; i < DataModel::EOriginDepthTypeQuantity; ++i ) {
			if ( !usedFlags[i] ) {
				SC_D.ui.cbDepthType->addItem(DataModel::EOriginDepthTypeNames::name(i), i);
				SC_D.ui.cbDepthType->setItemData(SC_D.ui.cbDepthType->count()-1, reducedColor, Qt::ForegroundRole);
			}
		}
	}

	vector<string> *locatorInterfaces = Seismology::LocatorInterfaceFactory::Services();
	if ( locatorInterfaces ) {
		for ( size_t i = 0; i < locatorInterfaces->size(); ++i )
			SC_D.ui.cbLocator->addItem((*locatorInterfaces)[i].c_str());
		delete locatorInterfaces;
	}

	int defaultLocatorIdx = SC_D.ui.cbLocator->findText(defaultLocator.c_str());
	if ( defaultLocatorIdx < 0 )
		defaultLocatorIdx = SC_D.ui.cbLocator->findText("LOCSAT");

	SC_D.ui.cbLocator->setCurrentIndex(defaultLocatorIdx);

	connect(SC_D.ui.cbLocator, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(locatorChanged(const QString &)));

	connect(SC_D.ui.cbLocatorProfile, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(locatorProfileChanged(const QString &)));

	connect(SC_D.ui.btnLocatorSettings, SIGNAL(clicked()),
	        this, SLOT(configureLocator()));

	locatorChanged(SC_D.ui.cbLocator->currentText());

	SC_D.minimumDepth = -999;

	try {
		// "locator.minimumDepth" preferred
		SC_D.minimumDepth = SCApp->configGetDouble("olv.locator.minimumDepth");
	}
	catch ( ... ) {
		try {
			SC_D.minimumDepth = SCApp->configGetDouble("locator.minimumDepth");
		}
		catch ( ... ) {}
	}

	try {
		SC_D.ui.btnCustom0->setText(SCApp->configGetString("button0").c_str());
	}
	catch ( ... ) {}

	try {
		SC_D.ui.btnCustom1->setText(SCApp->configGetString("button1").c_str());
	}
	catch ( ... ) {}

	connect(SC_D.ui.btnCustom0, SIGNAL(clicked()), this, SLOT(runScript0()));
	connect(SC_D.ui.btnCustom1, SIGNAL(clicked()), this, SLOT(runScript1()));

	try {
		vector<string> cols = SCApp->configGetStrings("olv.arrivalTable.visibleColumns");
		for ( int i = 0; i < ArrivalListColumns::Quantity; ++i )
			colVisibility[i] = false;

		for ( size_t i = 0; i < cols.size(); ++i ) {
			ArrivalListColumns v;
			if ( !v.fromString(cols[i]) ) {
				cerr << "ERROR: olv.arrivalTable.visibleColumns: invalid column name '"
				     << cols[i] << "' at index " << i << ", ignoring" << endl;
				continue;
			}

			colVisibility[v] = true;
		}
	}
	catch ( ... ) {}

	// Commit bags
	set<string> profiles;
	string customConfigPrefix = "olv.customCommits.";
	Seiscomp::Config::SymbolTable::iterator it = SCApp->configuration().symbolTable()->begin();
	for ( ; it != SCApp->configuration().symbolTable()->end(); ++it ) {
		const string &param = (*it)->name;
		if ( param.compare(0, customConfigPrefix.size(), customConfigPrefix) ) continue;
		size_t pos = param.find('.', customConfigPrefix.size());
		if ( pos == string::npos ) continue;
		string profile = param.substr(customConfigPrefix.size(), pos-customConfigPrefix.size());
		if ( profiles.find(profile) != profiles.end() ) continue;
		profiles.insert(profile);

		QLayout *toolBarLayout = SC_D.ui.frameActionsRight->layout();
		string prefix = customConfigPrefix + profile + ".";

		try {
			if ( !SCApp->configGetBool(prefix + "enable") )
				continue;
		}
		catch ( ...) {}

		CommitOptions customOptions;

		// Configure the commit+ button
		customOptions.init(prefix, nullptr);

		try {
			EventType et;
			if ( et.fromString(SCApp->configGetString(prefix + "eventType")) ) {
				customOptions.eventType = et;
			}
			else {
				QMessageBox::critical(this, "Error",
				                      QString("Invalid '%1eventType': %2")
				                      .arg(prefix.c_str())
				                      .arg(SCApp->configGetString("olv.commit.bulk.eventType").c_str()));
				continue;
			}
		}
		catch ( ... ) {}

		try {
			EventTypeCertainty etc;
			if ( etc.fromString(SCApp->configGetString(prefix + "eventTypeCertainty")) )
				customOptions.eventTypeCertainty = etc;
			else {
				QMessageBox::critical(this, "Error",
				                      QString("Invalid '%1eventTypeCertainty': %2")
				                      .arg(prefix.c_str())
				                      .arg(SCApp->configGetString("olv.commit.bulk.eventTypeCertainty").c_str()));
				continue;
			}
		}
		catch ( ... ) {}

		try {
			EvaluationStatus stat;
			string strStat = SCApp->configGetString(prefix + "originStatus");
			if ( strStat.empty() )
				customOptions.originStatus = OPT(EvaluationStatus)();
			else if ( stat.fromString(strStat) )
				customOptions.originStatus = OPT(EvaluationStatus)(stat);
			else {
				QMessageBox::critical(this, "Error",
				                      QString("Invalid '%1originStatus': %2")
				                      .arg(prefix.c_str())
				                      .arg(SCApp->configGetString("olv.commit.bulk.originStatus").c_str()));
				continue;
			}
		}
		catch ( ... ) {}

		try {
			customOptions.magnitudeType = SCApp->configGetString(prefix + "magnitudeType");
		}
		catch ( ... ) {}

		try {
			customOptions.eventName = SCApp->configGetString(prefix + "eventName");
		}
		catch ( ... ) {}

		try {
			customOptions.eventComment = SCApp->configGetString(prefix + "eventComment");
		}
		catch ( ... ) {}

		QToolButton *button = new QToolButton(SC_D.ui.toolButtonGroupBox);

		try {
			button->setText(SCApp->configGetString(prefix + "label").c_str());
		}
		catch ( ...) {
			button->setText(QString("Commit #%1").arg(profiles.size()));
		}

		try {
			QColor col = SCApp->configGetColor(prefix + "color", QColor());
			if ( col.isValid() ) {
				QPalette pal = button->palette();
				pal.setColor(QPalette::Button, col);
				button->setPalette(pal);
			}
		}
		catch ( ...) {}

		try {
			QColor col = SCApp->configGetColor(prefix + "colorText", QColor());
			if ( col.isValid() ) {
				QPalette pal = button->palette();
				pal.setColor(QPalette::ButtonText, col);
				button->setPalette(pal);
			}
		}
		catch ( ...) {}

		customOptions.valid = true;
		button->setProperty("customCommit", QVariant::fromValue<CommitOptions>(customOptions));

		try {
			if ( SCApp->configGetBool(prefix + "tooltip") )
				button->setToolTip(toString(customOptions));
		}
		catch ( ... ) {}

		toolBarLayout->addWidget(button);
		connect(button, SIGNAL(clicked()), this, SLOT(customCommit()));
	}

	resetCustomLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::~OriginLocatorView() {
	if ( SC_D.plotFilter ) {
		delete SC_D.plotFilter;
	}

	delete _d_ptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::locatorChanged(const QString &text) {
	SC_D.ui.cbLocatorProfile->clear();

	SC_D.locator = Seismology::LocatorInterfaceFactory::Create(SC_D.ui.cbLocator->currentText().toStdString().c_str());
	if ( !SC_D.locator ) {
		SC_D.ui.cbLocatorProfile->setEnabled(false);
		return;
	}

	SC_D.locator->init(SCApp->configuration());

	set<string> models;
	Seismology::LocatorInterface::IDList profiles = SC_D.locator->profiles();
	for ( Seismology::LocatorInterface::IDList::iterator it = profiles.begin();
	      it != profiles.end(); ++it ) {
		if ( models.find(*it) != models.end() ) continue;
		SC_D.ui.cbLocatorProfile->addItem(it->c_str());
	}

	int defaultIndex = SC_D.ui.cbLocatorProfile->findText(SC_D.defaultEarthModel.c_str());
	if ( defaultIndex >= 0 ) {
		SC_D.ui.cbLocatorProfile->setCurrentIndex(defaultIndex);
	}
	else {
		SC_D.ui.cbLocatorProfile->setCurrentIndex(0);
	}

	SC_D.ui.cbLocatorProfile->setEnabled(SC_D.ui.cbLocatorProfile->count() > 0);

	SC_D.ui.cbFixedDepth->setEnabled(SC_D.locator->supports(Seismology::LocatorInterface::FixedDepth));
	SC_D.ui.cbDistanceCutOff->setEnabled(SC_D.locator->supports(Seismology::LocatorInterface::DistanceCutOff));
	SC_D.ui.cbIgnoreInitialLocation->setEnabled(SC_D.locator->supports(Seismology::LocatorInterface::IgnoreInitialLocation));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::configureLocator() {
	if ( !SC_D.locator ) {
		QMessageBox::critical(this, "Locator settings",
		                      "No locator selected.");
		return;
	}

	Seismology::LocatorInterface::IDList params = SC_D.locator->parameters();
	if ( params.empty() ) {
		QMessageBox::information(this, "Locator settings",
		                         QString("%1 does not provide any "
		                                 "parameters to adjust.")
		                         .arg(SC_D.locator->name().c_str()));
		return;
	}

	LocatorSettings dlg;
	dlg.setWindowTitle(QString("%1 settings").arg(SC_D.locator->name().c_str()));

	for ( auto param : params ) {
		dlg.addRow(param.c_str(), SC_D.locator->parameter(param).c_str());
	}

	if ( dlg.exec() != QDialog::Accepted ) {
		return;
	}

	LocatorSettings::ContentList res = dlg.content();
	for ( auto it : res ) {
		SC_D.locator->setParameter(it.first.toStdString(), it.second.toStdString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changePlotFilter() {
	if ( !SC_D.plotFilterSettings ) {
		SC_D.plotFilterSettings = new DiagramFilterSettingsDialog(this);
	}

	if ( SC_D.plotFilterSettings->exec() == QDialog::Rejected ) {
		return;
	}

	setPlotFilter(SC_D.plotFilterSettings->createFilter());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPlotFilter(DiagramFilterSettingsDialog::Filter *f) {
	if ( SC_D.plotFilter ) {
		delete SC_D.plotFilter;
	}

	SC_D.plotFilter = f;
	applyPlotFilter();

	if ( SC_D.plotFilter ) {
		SC_D.ui.labelPlotFilter->setText("<a href=\"filter\">active</a>");
	}
	else {
		SC_D.ui.labelPlotFilter->setText("<a href=\"filter\">not active</a>");
	}

	SC_D.ui.labelPlotFilter->setCursor(Qt::PointingHandCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::applyPlotFilter() {
	if ( SC_D.plotFilter == nullptr ) {
		for ( int i = 0; i < SC_D.residuals->count(); ++i )
			SC_D.residuals->showValue(i);
	}
	else {
		for ( int i = 0; i < SC_D.residuals->count(); ++i )
			SC_D.residuals->showValue(i, SC_D.plotFilter->accepts(SC_D.residuals, i));
	}

	SC_D.residuals->updateBoundingRect();
	QRectF rect = SC_D.residuals->boundingRect();
	rect.setLeft(std::min(0.0, double(rect.left())));
	adjustResidualsRect(rect);
	SC_D.residuals->setDisplayRect(rect);
	SC_D.residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript(const QString &script, const QString &name) {
	QString cmd = QString("%1 %2").arg(script).arg(SC_D.currentOrigin->publicID().c_str());
	if ( SC_D.baseEvent ) {
		cmd += QString(" %1").arg(SC_D.baseEvent->publicID().c_str());
	}
	SEISCOMP_DEBUG("Executing script %s", cmd.toStdString().c_str());

	// start as background process w/o any communication channel
	if ( !QT_PROCESS_STARTDETACHED(cmd) ) {
		SEISCOMP_ERROR("Failed executing script %s", cmd.toStdString().c_str());
		QMessageBox::warning(this, name, tr("Can't execute script"));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript0() {
	runScript(SC_D.script0.c_str(), SC_D.ui.btnCustom0->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript1() {
	runScript(SC_D.script1.c_str(), SC_D.ui.btnCustom1->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::locatorProfileChanged(const QString &text) {
	if ( SC_D.locator ) {
		SC_D.locator->setProfile(text.toStdString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setMagnitudeCalculationEnabled(bool e) {
	SC_D.ui.btnMagnitudes->setVisible(e);
	if ( !e )
		SC_D.ui.btnMagnitudes->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorMap *OriginLocatorView::map() const {
	return SC_D.map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::closeEvent(QCloseEvent *e) {
	if ( SC_D.recordView ) {
		SC_D.recordView->close();
	}

	QWidget::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::residualsSelected() {
	QVector<int> selectedIds = SC_D.residuals->getSelectedValues();
	int startIndex = 0;
	for ( int i = 0; i < selectedIds.count(); ++i ) {
		for ( int j = startIndex; j < selectedIds[i]; ++j ) {
			SC_D.modelArrivals.setData(SC_D.modelArrivals.index(j, USED), 0, RestoreRole);
		}

		// Restore default arrival usage
		SC_D.modelArrivals.setData(SC_D.modelArrivals.index(selectedIds[i], USED),
		                           1, RestoreRole);

		startIndex = selectedIds[i]+1;
	}

	for ( int j = startIndex; j < SC_D.modelArrivals.rowCount(); ++j ) {
		SC_D.modelArrivals.setData(SC_D.modelArrivals.index(j, USED), 0, RestoreRole);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::hoverArrival(int id) {
	auto w = static_cast<QWidget*>(sender());
	if ( id == -1 ) {
		w->setToolTip("");
	}
	else {
		QString txt = QString("%1.%2-%3")
		              .arg(SC_D.modelArrivals.data(SC_D.modelArrivals.index(id, NETWORK), Qt::DisplayRole).toString())
		              .arg(SC_D.modelArrivals.data(SC_D.modelArrivals.index(id, STATION), Qt::DisplayRole).toString())
		              .arg(SC_D.modelArrivals.data(SC_D.modelArrivals.index(id, PHASE), Qt::DisplayRole).toString());
		QString method = SC_D.modelArrivals.data(SC_D.modelArrivals.index(id, METHOD), Qt::DisplayRole).toString();

		if ( !method.isEmpty() ) {
			txt += QString(" (%1)").arg(method);
		}
		QPointF p = SC_D.residuals->value(id);
		txt += QString("\n%1 / %2").arg(p.x()).arg(p.y());
		w->setToolTip(txt);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrival(int id) {
	if ( id >= 0 && SC_D.currentOrigin ) {
		if ( SC_D.recordView ) {
			Arrival *ar = SC_D.currentOrigin->arrival(id);
			Pick *pick = Pick::Find(ar->pickID());
			if ( pick )
				SC_D.recordView->selectTrace(pick->waveformID());
		}

		QModelIndex idx = SC_D.modelArrivalsProxy->mapFromSource(SC_D.modelArrivals.index(id, 0));
		SC_D.ui.tableArrivals->setCurrentIndex(idx);
		SC_D.ui.tableArrivals->scrollTo(idx);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectStation(const std::string &net,
                                      const std::string &code) {
	if ( SC_D.recordView ) SC_D.recordView->selectTrace(net, code);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectRow(const QModelIndex &current, const QModelIndex&) {
	QModelIndex idx = SC_D.modelArrivalsProxy->mapToSource(current);
	if ( idx.row() >= 0 && SC_D.currentOrigin && SC_D.recordView ) {
		Arrival *ar = SC_D.currentOrigin->arrival(idx.row());
		Pick *pick = Pick::Find(ar->pickID());
		if ( pick )
			SC_D.recordView->selectTrace(pick->waveformID());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::adjustResidualsRect(QRectF& rect) {
	qreal width = rect.width();

	// left
	if ( rect.left() > 0 ) {
		rect.setLeft(diagramFloor(rect.left(), width));
	}
	else {
		rect.setLeft(0.0);
	}

	// right
	qreal maxRight = 360.0;
	if ( SCScheme.unit.distanceInKM && (
	         SC_D.plotTab->currentIndex() == PT_DISTANCE ||
	         SC_D.plotTab->currentIndex() == PT_TRAVELTIME ||
	         SC_D.plotTab->currentIndex() == PT_MOVEOUT ||
	         SC_D.plotTab->currentIndex() == PT_POLAR ) ) {
		maxRight = ceil(Math::Geo::deg2km(maxRight));
	}
	if ( rect.right() < maxRight ) {
		rect.setRight(diagramCeil(rect.right(), width));
	}
	else {
		rect.setRight(maxRight);
	}

	// polar and fm plots: fixed values for top/bottom
	if ( SC_D.plotTab->currentIndex() == PT_POLAR ||
	     SC_D.plotTab->currentIndex() == PT_FM ) {
		rect.setTop(0);
		rect.setBottom(360);

		return;
	}

	// travel time plot: top starts at 0 unless negative values are present
	if ( SC_D.plotTab->currentIndex() == PT_TRAVELTIME ) {
		qreal vMargin = rect.height() * 0.1;
		rect.setTop(rect.top() < 0.0 ? rect.top() - vMargin : 0.0);
		rect.setBottom(rect.bottom() + vMargin);
		return;
	}

	// remaining plots
	qreal maxResidual = max(abs(rect.bottom()), abs(rect.top())) * 1.1;
	rect.setTop(-maxResidual);
	rect.setBottom(maxResidual);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::plotTabChanged(int tab) {
	static_cast<PlotWidget*>(SC_D.residuals)->setCustomDraw(false);

	auto distanceLabel = []() {
		return QString("%1 distance (%2)")
		       .arg(SCScheme.distanceHypocentral ? "Hyp." : "Epi.")
		       .arg(SCScheme.unit.distanceInKM ? "km" :"°");
	};

	// Distance / Residual
	if ( tab == PT_DISTANCE ) {
		SC_D.residuals->setMarkerDistance(10, 1);
		SC_D.residuals->setType(DiagramWidget::Rectangular);
		SC_D.residuals->setIndicies(PC_DISTANCE,PC_RESIDUAL);
		SC_D.residuals->setAbscissaName(distanceLabel());
		SC_D.residuals->setOrdinateName("Residual (s)");
	}
	// Azimuth / Residual
	else if ( tab == PT_AZIMUTH ) {
		SC_D.residuals->setMarkerDistance(10, 1);
		SC_D.residuals->setType(DiagramWidget::Rectangular);
		SC_D.residuals->setIndicies(PC_AZIMUTH,PC_RESIDUAL);
		SC_D.residuals->setAbscissaName("Azimuth (°)");
		SC_D.residuals->setOrdinateName("Residual (s)");
	}
	// Distance / TravelTime
	else if ( tab == PT_TRAVELTIME ) {
		SC_D.residuals->setMarkerDistance(10, 10);
		SC_D.residuals->setType(DiagramWidget::Rectangular);
		SC_D.residuals->setIndicies(PC_DISTANCE,PC_TRAVELTIME);
		SC_D.residuals->setAbscissaName(distanceLabel());
		SC_D.residuals->setOrdinateName("TravelTime (s)");
	}
	else if ( tab == PT_MOVEOUT ) {
		SC_D.residuals->setMarkerDistance(10, 10);
		SC_D.residuals->setType(DiagramWidget::Rectangular);
		SC_D.residuals->setIndicies(PC_DISTANCE,PC_REDUCEDTRAVELTIME);
		SC_D.residuals->setAbscissaName(distanceLabel());
		SC_D.residuals->setOrdinateName(QString("Tred = T-d/%1 km/s (s)").arg(SC_D.config.reductionVelocityP));
	}
	else if ( tab == PT_POLAR ) {
		SC_D.residuals->setType(DiagramWidget::Spherical);
		SC_D.residuals->setIndicies(PC_DISTANCE,PC_AZIMUTH);
	}
	else if ( tab == PT_FM ) {
		static_cast<PlotWidget*>(SC_D.residuals)->setCustomDraw(true);
		SC_D.residuals->setType(DiagramWidget::Spherical);
		SC_D.residuals->setIndicies(PC_FMDIST,PC_FMAZI);
	}

	QRectF rect = SC_D.residuals->boundingRect();

	rect.setLeft(std::min(0.0, double(rect.left())));
	if ( rect.bottom() == rect.top() ) {
		rect.setTop(rect.bottom() + 1.0);
	}

	if ( tab == PT_AZIMUTH ) {
		rect.setRight(360.0);
	}

	adjustResidualsRect(rect);

	SC_D.residuals->setDisplayRect(rect);
	SC_D.residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changeArrival(int id, bool state) {
	QModelIndex idx = SC_D.modelArrivals.index(id, USED);
	SC_D.modelArrivals.setData(idx, state ? 1 : 0, RestoreRole);

	SC_D.residuals->setValueSelected(id, state);
	SC_D.map->setArrivalState(id, state);

	if ( SC_D.toolMap ) {
		SC_D.toolMap->setArrivalState(id, state);
	}

	if ( SC_D.recordView ) {
		SC_D.recordView->setArrivalState(id, state);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changeArrivalEnableState(int id,bool state) {
	//changeArrival(id, state);
	SC_D.modelArrivals.setRowEnabled(id, state);
	SC_D.residuals->setValueEnabled(id, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::artificialOriginRequested(double lat, double lon,
                                                  double depth,
                                                  Seiscomp::Core::Time time) {
	createArtificialOrigin(QPointF(lon, lat), depth, time, QCursor::pos());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::zoomMap() {
	if ( SC_D.toolMap != nullptr ) {
		SC_D.toolMap->activateWindow();
		SC_D.toolMap->raise();
		return;
	}

	SC_D.toolMap = new OriginLocatorMap(SC_D.maptree.get(), this, Qt::Window);
	SC_D.toolMap->setAttribute(Qt::WA_DeleteOnClose);
	connect(SC_D.toolMap, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(mapKeyPressed(QKeyEvent*)));
	connect(SC_D.toolMap, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	connect(SC_D.toolMap, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	if ( SC_D.currentOrigin ) {
		SC_D.toolMap->setOrigin(SC_D.currentOrigin.get());
		SC_D.toolMap->canvas().displayRect(QRectF(SC_D.currentOrigin->longitude()-20, SC_D.currentOrigin->latitude()-20, 40, 40));
		SC_D.toolMap->setDrawStations(SC_D.map->drawStations());
	}
	SC_D.toolMap->setWindowTitle("OriginLocator::Map");
	SC_D.toolMap->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::mapKeyPressed(QKeyEvent* e) {
	if ( SC_D.toolMap == nullptr ) return;

	switch ( e->key() ) {
		case Qt::Key_Escape:
			SC_D.toolMap->close();
			break;
		case Qt::Key_F9:
			drawStations(!SC_D.map->drawStations());
			break;
		case Qt::Key_F11:
			if ( SC_D.toolMap->isFullScreen() )
				SC_D.toolMap->showNormal();
			else
				SC_D.toolMap->showFullScreen();
			break;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::objectDestroyed(QObject* o) {
	if ( o == SC_D.toolMap ) {
		SC_D.toolMap = nullptr;
		//SC_D.ui.btnZoom->setEnabled(true);
	}

	if ( o == SC_D.recordView ) {
		//std::cout << "Number of objects after: " << Core::BaseObject::ObjectCount() << std::endl;
		SC_D.recordView = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::drawStations(bool enable) {
	SC_D.map->setDrawStations(enable);
	SC_D.map->update();

	if ( SC_D.toolMap ) {
		SC_D.toolMap->setDrawStations(enable);
		SC_D.toolMap->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::drawStationAnnotations(bool enable) {
	SC_D.map->setDrawStationAnnotations(enable);
	SC_D.map->update();

	if ( SC_D.toolMap ) {
		SC_D.toolMap->setDrawStationAnnotations(enable);
		SC_D.toolMap->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::readPicks(Origin* o) {
	if ( SC_D.blockReadPicks ) return;

	SC_D.blockReadPicks = true;

	if ( SC_D.reader ) {
		if ( o->arrivalCount() == 0 ) {
			SC_D.reader->loadArrivals(o);
		}

		if ( o->magnitudeCount() == 0 ) {
			SC_D.reader->loadMagnitudes(o);
		}

		for ( size_t i = 0; i < o->magnitudeCount(); ++i ) {
			if ( o->magnitude(i)->commentCount() == 0 ) {
				SC_D.reader->loadComments(o->magnitude(i));
			}
			if ( o->magnitude(i)->stationMagnitudeContributionCount() == 0 ) {
				SC_D.reader->loadStationMagnitudeContributions(o->magnitude(i));
			}
		}

		if ( o->stationMagnitudeCount() == 0 ) {
			SC_D.reader->loadStationMagnitudes(o);
		}

		PickMap originPicks;
		std::vector<PickPtr> tmpPicks;

		bool missing = false;
		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			std::string pickID = o->arrival(i)->pickID();
			if ( Pick::Find(pickID) == nullptr ) {
				missing = true;
				break;
			}
		}

		if ( missing ) {
			QProgressDialog progress(this);
			progress.setWindowTitle(tr("Please wait..."));
			progress.setRange(0, o->arrivalCount());
			progress.setLabelText(tr("Loading picks..."));
			progress.setCancelButton(nullptr);
			DatabaseIterator it = SC_D.reader->getPicks(o->publicID());

			while ( *it ) {
				if ( !it.cached() ) {
					tmpPicks.push_back(Pick::Cast(*it));
				}
				++it;
				progress.setValue(progress.value()+1);
			}
		}

		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			std::string pickID = o->arrival(i)->pickID();

			PickMap::iterator it = SC_D.associatedPicks.find(pickID);
			if ( it != SC_D.associatedPicks.end() ) {
				originPicks[pickID] = it->second;
				continue;
			}

			// try to find the pick somewhere in the client memory
			PickPtr pick = Pick::Find(pickID);
			if ( pick ) {
				originPicks[pickID] = pick;
			}
			else {
				auto pick = static_cast<Pick*>(SC_D.reader->getObject(Pick::TypeInfo(), pickID));
				if ( pick ) {
					originPicks[pickID] = pick;
				}
			}
		}

		SC_D.associatedPicks = originPicks;
	}

	SC_D.blockReadPicks = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateBlinkState() {
	//--_relocateBlinkCounter;
	if ( SC_D.blinkCounter <= 0 ) {
		SC_D.blinkCounter = 0;
		SC_D.blinker = 0;
		SC_D.blinkTimer.stop();
	}

	if ( !SC_D.blinkWidget ) {
		return;
	}

	QPalette pal = SC_D.blinkWidget->palette();

	int percent = (int)(25 * sin(8*(SC_D.blinker++) * 2 * M_PI / 100 - M_PI/2) + 25);

	QColor blink = blend(SC_D.blinkColor, qApp->palette().color(QPalette::Button), percent);
	pal.setColor(QPalette::Button, blink);
	SC_D.blinkWidget->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::startBlinking(QColor c, QWidget *w) {
	if ( SC_D.blinkWidget != nullptr && SC_D.blinkWidget != w )
		stopBlinking();

	SC_D.blinkCounter = 50;
	SC_D.blinkColor = c;
	SC_D.blinker = 0;
	SC_D.blinkWidget = w;
	SC_D.blinkTimer.start(40);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::stopBlinking() {
	SC_D.blinkCounter = 0;
	updateBlinkState();
	SC_D.blinkWidget = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setCreatedOrigin(Seiscomp::DataModel::Origin* o) {
	ObjectChangeList<DataModel::Pick> changedPicks;
	SC_D.recordView->getChangedPicks(changedPicks);
	SEISCOMP_DEBUG("received new origin with %lu manual picks", (unsigned long)changedPicks.size());

	startBlinking(QColor(255,128,0), SC_D.ui.btnRelocate);
	SC_D.ui.btnRelocate->setFocus();

	SC_D.ui.btnCommit->setEnabled(true);
	SC_D.ui.btnCommit->setText("Commit");
//	SC_D.ui.btnCommit->setMenu(SC_D.baseEvent?_commitMenu:nullptr);
	SC_D.localOrigin = true;

	// Update pick cache
	for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
		Pick* p = Pick::Find(o->arrival(i)->pickID());
		if ( p ) SC_D.associatedPicks[p->publicID()] = p;
	}

	pushUndo();
	SC_D.blockReadPicks = true;
	updateOrigin(o);
	SC_D.blockReadPicks = false;

	//computeMagnitudes();
	SC_D.ui.btnMagnitudes->setEnabled(true);

	SC_D.changedPicks.insert(changedPicks.begin(), changedPicks.end());

	emit newOriginSet(o, SC_D.baseEvent.get(), SC_D.localOrigin, false);
	emit requestRaise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setConfig(const Config &c) {
	bool updateTakeOffAngles = SC_D.config.computeMissingTakeOffAngles != c.computeMissingTakeOffAngles;

	SC_D.config = c;
	SC_D.residuals->setDrawGridLines(SC_D.config.drawGridLines);
	SC_D.map->setDrawStationLines(c.drawMapLines);

	for ( int i = 0; i < SC_D.residuals->count(); ++i ) {
		if ( SC_D.residuals->isValueValid(i,PC_DISTANCE) && SC_D.residuals->isValueValid(i,PC_TRAVELTIME) ) {
			if ( SCScheme.unit.distanceInKM )
				SC_D.residuals->setValue(i,PC_REDUCEDTRAVELTIME, SC_D.residuals->value(i,PC_TRAVELTIME) - SC_D.residuals->value(i,PC_DISTANCE)/SC_D.config.reductionVelocityP);
			else
				SC_D.residuals->setValue(i,PC_REDUCEDTRAVELTIME, SC_D.residuals->value(i,PC_TRAVELTIME) - Math::Geo::deg2km(SC_D.residuals->value(i,PC_DISTANCE))/SC_D.config.reductionVelocityP);
		}
		else {
			SC_D.residuals->setValue(i, PC_REDUCEDTRAVELTIME, 0.0);
			SC_D.residuals->setValueValid(i, PC_REDUCEDTRAVELTIME, false);
		}

		if ( updateTakeOffAngles && SC_D.currentOrigin ) {
			char phase = Util::getShortPhaseName(SC_D.currentOrigin->arrival(i)->phase().code());
			PlotWidget::PolarityType polarity = (PlotWidget::PolarityType)SC_D.residuals->value(i, PC_POLARITY);

			SC_D.residuals->setValue(i, PC_FMDIST, 0.0);
			SC_D.residuals->setValue(i, PC_FMAZI, 0.0);
			SC_D.residuals->setValueValid(i, PC_FMDIST, false);
			SC_D.residuals->setValueValid(i, PC_FMAZI, false);

			if ( SC_D.residuals->isValueValid(i, PC_DISTANCE) &&
				 SC_D.residuals->isValueValid(i, PC_AZIMUTH) &&
				 phase == 'P' ) {

				double beta;
				bool hasTakeOff;
				try {
					beta = SC_D.currentOrigin->arrival(i)->takeOffAngle();
					hasTakeOff = true;
				}
				catch ( ... ) {
					hasTakeOff = false;
				}

				if ( !hasTakeOff && SC_D.config.computeMissingTakeOffAngles ) {
					double lat, lon;
					double azi = SC_D.residuals->value(i, PC_AZIMUTH);

					Math::Geo::delandaz2coord(
						SC_D.currentOrigin->arrival(i)->distance(), azi,
						SC_D.currentOrigin->latitude(), SC_D.currentOrigin->longitude(),
						&lat, &lon
					);

					try {
						TravelTime ttt = SC_D.ttTable.computeFirst(
							SC_D.currentOrigin->latitude(), SC_D.currentOrigin->longitude(),
							SC_D.currentOrigin->depth(), lat, lon
						);

						beta = ttt.takeoff;
						SC_D.modelArrivals.setTakeOffAngle(i, beta);

						hasTakeOff = true;
					}
					catch ( ... ) {}
				}

				if ( hasTakeOff &&
					 static_cast<PlotWidget*>(SC_D.residuals)->shape(polarity).shown ) {
					double azi = SC_D.residuals->value(i, PC_AZIMUTH);

					if ( beta > 90 ) {
						beta = 180-beta;
						azi = azi-180;
						if ( azi < 0 ) azi += 360;
					}

					beta = sqrt(2.0) * sin(0.5*deg2rad(beta));

					SC_D.residuals->setValue(i, PC_FMAZI, azi);
					SC_D.residuals->setValue(i, PC_FMDIST, beta);
					SC_D.residuals->setValueValid(i, PC_FMDIST, true);
					SC_D.residuals->setValueValid(i, PC_FMAZI, true);
				}
			}
		}
	}

	if ( SC_D.plotTab->currentIndex() == PT_MOVEOUT ) {
		SC_D.residuals->updateBoundingRect();
		QRectF rect = SC_D.residuals->boundingRect();
		rect.setLeft(std::min(0.0, double(rect.left())));
		adjustResidualsRect(rect);
		SC_D.residuals->setDisplayRect(rect);
		SC_D.residuals->setOrdinateName(QString("TTred >x/%1").arg(SC_D.config.reductionVelocityP));
	}

	SC_D.residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OriginLocatorView::Config &OriginLocatorView::config() const {
	return SC_D.config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addLocalPick(Seiscomp::DataModel::Pick *pick) {
	SC_D.changedPicks.insert(std::pair<DataModel::PickPtr, bool>(pick, true));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPickerConfig(const PickerView::Config &c) {
	SC_D.pickerConfig = c;

	if ( SC_D.recordView )
		SC_D.recordView->setConfig(SC_D.pickerConfig);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const PickerView::Config& OriginLocatorView::pickerConfig() const {
	return SC_D.pickerConfig;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	SC_D.reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPickerView(PickerView* picker) {
	SC_D.recordView = picker;

	connect(SC_D.recordView, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(SC_D.recordView, SIGNAL(arrivalEnableStateChanged(int,bool)), this, SLOT(changeArrivalEnableState(int,bool)));
	connect(SC_D.recordView, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	connect(SC_D.recordView, SIGNAL(originCreated(Seiscomp::DataModel::Origin*)),
	        this, SLOT(setCreatedOrigin(Seiscomp::DataModel::Origin*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addObject(const QString &parentID, Seiscomp::DataModel::Object *o) {
	if ( SC_D.currentOrigin ) {
		OriginReferencePtr ref = OriginReference::Cast(o);
		if ( ref ) {
			if ( SC_D.baseEvent ) {
				if ( parentID == SC_D.baseEvent->publicID().c_str() ) {
					OriginPtr o = Origin::Find(ref->originID());
					if ( o && (o->arrivalCount() > SC_D.currentOrigin->arrivalCount()) )
						startBlinking(QColor(128,255,0), SC_D.ui.btnImportAllArrivals);

					if ( ref->originID() == SC_D.currentOrigin->publicID() )
						emit baseEventSet();
				}
				else if ( ref->originID() == SC_D.currentOrigin->publicID() ) {
					// Current origin is associated with another event, load it
					EventPtr evt = Event::Find(parentID.toStdString());
					if ( !evt && SC_D.reader )
						evt = Event::Cast(SC_D.reader->loadObject(Event::TypeInfo(), parentID.toStdString()));

					if ( evt ) {
						QMessageBox::information(this, tr("Event change"),
						                         tr("The current origin was associated to another event than the current.\n"
						                            "Event %1 is being loaded.").arg(parentID));
						setBaseEvent(evt.get());
						emit baseEventSet();
					}
					else {
						QMessageBox::warning(this, tr("Event change"),
						                     tr("The current origin was associated to another event than the current.\n"
						                        "Unfortunately event %1 could not be loaded.").arg(parentID));
						emit baseEventRejected();
					}
				}
			}
			else if ( ref->originID() == SC_D.currentOrigin->publicID() ) {
				// Set base event
			}
		}

		if ( SC_D.displayComment ) {
			if ( parentID == SC_D.currentOrigin->publicID().c_str() ) {
				Comment *comment = Comment::Cast(o);
				if ( comment && comment->id() == SC_D.displayCommentID )
					SC_D.ui.labelComment->setText(comment->text().c_str());
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateObject(const QString& parentID, Seiscomp::DataModel::Object *o) {
	if ( SC_D.baseEvent) {
		Event *evt = Event::Cast(o);
		if ( evt && evt->publicID() == SC_D.baseEvent->publicID() ) {
			if ( evt->preferredFocalMechanismID() != SC_D.preferredFocMech ) {
				// Trigger preferred FM update
				setBaseEvent(SC_D.baseEvent.get());
			}
		}
	}

	if ( SC_D.currentOrigin ) {
		Origin *org = Origin::Cast(o);
		if ( org && org->publicID() == SC_D.currentOrigin->publicID() )
			updateContent();
		else if ( SC_D.displayComment ) {
			if ( parentID == SC_D.currentOrigin->publicID().c_str() ) {
				Comment *comment = Comment::Cast(o);
				if ( comment && comment->id() == SC_D.displayCommentID )
					SC_D.ui.labelComment->setText(comment->text().c_str());
			}
		}
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setOrigin(Seiscomp::DataModel::Origin* o) {
	setOrigin(o, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setBaseEvent(DataModel::Event *e) {
	if ( !SC_D.baseEvent && e ) {
		SC_D.baseEvent = e;
		emit baseEventSet();
	}
	else
		SC_D.baseEvent = e;

	SC_D.preferredFocMech = string();

	//static_cast<PlotWidget*>(SC_D.residuals)->setPreferredFM(355,60,41);
	//return;

	if ( SC_D.baseEvent ) {
		SC_D.ui.labelEventID->setText(SC_D.baseEvent->publicID().c_str());
		SC_D.ui.labelEventID->setToolTip(SC_D.baseEvent->publicID().c_str());
	}
	else {
		SC_D.ui.labelEventID->setText("-");
		SC_D.ui.labelEventID->setToolTip("");

		static_cast<PlotWidget*>(SC_D.residuals)->set(90,90,0);
		static_cast<PlotWidget*>(SC_D.residuals)->resetPreferredFM();

		return;
	}

	SC_D.preferredFocMech = e->preferredFocalMechanismID();

	DataModel::FocalMechanismPtr fm = DataModel::FocalMechanism::Find(SC_D.preferredFocMech);
	if ( !fm && !e->preferredFocalMechanismID().empty() && SC_D.reader )
		fm = FocalMechanism::Cast(SC_D.reader->getObject(FocalMechanism::TypeInfo(), SC_D.preferredFocMech));

	if ( !fm ) {
		static_cast<PlotWidget*>(SC_D.residuals)->set(90,90,0);
		static_cast<PlotWidget*>(SC_D.residuals)->resetPreferredFM();
		return;
	}

	try {
		static_cast<PlotWidget*>(SC_D.residuals)->setPreferredFM(
			fm->nodalPlanes().nodalPlane1().strike(),
			fm->nodalPlanes().nodalPlane1().dip(),
			fm->nodalPlanes().nodalPlane1().rake()
		);

		static_cast<PlotWidget*>(SC_D.residuals)->set(
			fm->nodalPlanes().nodalPlane1().strike(),
			fm->nodalPlanes().nodalPlane1().dip(),
			fm->nodalPlanes().nodalPlane1().rake()
		);
	}
	catch ( ... ) {
		static_cast<PlotWidget*>(SC_D.residuals)->set(90,90,0);
		static_cast<PlotWidget*>(SC_D.residuals)->resetPreferredFM();
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::setOrigin(DataModel::Origin* o, DataModel::Event* e,
                                  bool local) {
	if ( SC_D.currentOrigin == o ) {
		if ( SC_D.baseEvent != e )
			setBaseEvent(e);
		return true;
	}

	if ( !SC_D.undoList.isEmpty() ) {
		if ( QMessageBox::question(this, "Show origin",
		                           tr("You have uncommitted modifications.\n"
		                              "When setting the new origin your modifications get lost.\n"
		                              "Do you really want to continue?"),
		                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
			return false;
	}

	// Reset plot filter if a new event has been loaded
	if ( (e != nullptr) && SC_D.baseEvent != e ) {
		setPlotFilter(nullptr);
	}

	stopBlinking();

	SC_D.changedPicks.clear();
	SC_D.baseOrigin = o;
	setBaseEvent(e);

	SC_D.undoList.clear();
	SC_D.redoList.clear();

	SC_D.ui.btnCommit->setText(local?"Commit":"Confirm");
//	SC_D.ui.btnCommit->setMenu(SC_D.baseEvent?_commitMenu:nullptr);

	// Disable distance cutoff when a new origin has been
	// set from external.
	SC_D.ui.cbDistanceCutOff->setChecked(false);
	SC_D.ui.cbIgnoreInitialLocation->setChecked(false);

	emit undoStateChanged(!SC_D.undoList.isEmpty());
	emit redoStateChanged(!SC_D.redoList.isEmpty());

	SC_D.ui.btnImportAllArrivals->setEnabled(true);

	try {
		if ( o && o->evaluationMode() == AUTOMATIC ) {
			SC_D.ui.btnCommit->setEnabled(false);
		}
		else {
			SC_D.ui.btnCommit->setEnabled(true);
		}
	}
	catch ( ... ) {
		SC_D.ui.btnCommit->setEnabled(false);
	}

	SC_D.blockReadPicks = false;
	SC_D.localOrigin = local;

	updateOrigin(o);

	if ( SC_D.recordView ) {
		SC_D.recordView->setOrigin(o, -5*60, 30*60);

		for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
			QModelIndex idx = SC_D.modelArrivals.index(i, USED);
			SC_D.recordView->setArrivalState(i, SC_D.modelArrivals.data(idx, UsedRole).toInt());
		}
	}

	emit newOriginSet(o, SC_D.baseEvent.get(), SC_D.localOrigin, false);
	SC_D.ui.btnMagnitudes->setEnabled(false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::clear() {
	stopBlinking();

	SC_D.associatedPicks.clear();
	SC_D.originPicks.clear();
	SC_D.changedPicks.clear();
	SC_D.baseOrigin = nullptr;
	SC_D.baseEvent = nullptr;

	SC_D.undoList.clear();
	SC_D.redoList.clear();

	SC_D.ui.btnCommit->setText("Confirm");
	SC_D.ui.btnCommit->setMenu(nullptr);

	emit undoStateChanged(false);
	emit redoStateChanged(false);

	SC_D.ui.btnImportAllArrivals->setEnabled(false);
	SC_D.ui.btnCommit->setEnabled(false);

	SC_D.blockReadPicks = false;
	resetCustomLabels();
	updateOrigin(nullptr);

	if ( SC_D.recordView )
		SC_D.recordView->close();

	SC_D.ui.btnMagnitudes->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateOrigin(Seiscomp::DataModel::Origin* o) {
	if ( SC_D.currentOrigin == o ) return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( o ) {
		readPicks(o);
	}

	SC_D.currentOrigin = o;
	SC_D.modelArrivals.setOrigin(o);

	updateContent();

	if ( SC_D.currentOrigin ) {
		for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
			Arrival *arrival = SC_D.currentOrigin->arrival(i);
			if ( !Client::Inventory::Instance()->getStation(Pick::Find(arrival->pickID())) ) {
				changeArrivalEnableState(i, false);
			}
		}

		bool presetLocator = false;
		try {
			presetLocator = SCApp->configGetBool("olv.locator.presetFromOrigin");
		}
		catch ( ... ) {}

		if ( presetLocator ) {
			// Preset the locator type and profile
			if ( !SC_D.localOrigin ) {
				// Do not change the profile if the origin has been relocated
				// locally.
				int idx = SC_D.ui.cbLocator->findText(SC_D.currentOrigin->methodID().c_str());
				if ( idx >= 0 ) {
					SC_D.ui.cbLocator->setCurrentIndex(idx);

					idx = SC_D.ui.cbLocatorProfile->findText(SC_D.currentOrigin->earthModelID().c_str());
					if ( idx >= 0 ) {
						SC_D.ui.cbLocatorProfile->setCurrentIndex(idx);
					}
				}
				else {
					std::string defaultLocator = "LOCSAT";
					try {
						defaultLocator = SCApp->configGetString("olv.locator.interface");
					}
					catch ( ... ) {
						try {
							defaultLocator = SCApp->configGetString("olv.locator");
						}
						catch ( ... ) {}
					}

					int defaultLocatorIdx = SC_D.ui.cbLocator->findText(defaultLocator.c_str());
					if ( defaultLocatorIdx < 0 ) {
						defaultLocatorIdx = SC_D.ui.cbLocator->findText("LOCSAT");
						if ( defaultLocatorIdx < 0 ) {
							defaultLocatorIdx = 0;
						}
					}

					if ( defaultLocatorIdx != SC_D.ui.cbLocator->currentIndex() ) {
						SC_D.ui.cbLocator->setCurrentIndex(defaultLocatorIdx);
					}
					else {
						locatorChanged(SC_D.ui.cbLocator->currentText());
					}
				}
			}

			// Preset fixed depth
			try {
				SC_D.ui.cbFixedDepth->setChecked(quantityUncertainty(SC_D.currentOrigin->depth()) == 0.0);
			}
			catch ( ... ) {
				SC_D.ui.cbFixedDepth->setChecked(false);
			}

			// Preset depth type
			try {
				int idx = SC_D.ui.cbDepthType->findData(SC_D.currentOrigin->depthType().toInt());
				SC_D.ui.cbDepthType->setCurrentIndex(idx > 0 ? idx : 1);
			}
			catch ( ValueException & ) {
				// No depth type set: use the defaul behaviour
			}
		}
	}

	SC_D.residuals->setEnabled(SC_D.currentOrigin != nullptr);

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::resetCustomLabels() {
	for ( auto it = SC_D.scriptLabelMap.begin(); it != SC_D.scriptLabelMap.end(); ++it ) {
		it.value().first->setEnabled(false);
		it.value().second->setText("");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateContent() {
	SC_D.residuals->clear();
	//SC_D.ui.tableArrivals->setModel(&SC_D.modelArrivals);

	if ( SC_D.ui.tableArrivals->selectionModel() ) {
		delete SC_D.ui.tableArrivals->selectionModel();
	}

	if ( SC_D.ui.tableArrivals->model() ) {
		delete SC_D.ui.tableArrivals->model();
	}

	SC_D.modelArrivalsProxy = new ArrivalsSortFilterProxyModel(this);
	SC_D.modelArrivalsProxy->setSourceModel(&SC_D.modelArrivals);
	SC_D.ui.tableArrivals->setModel(SC_D.modelArrivalsProxy);
	connect(SC_D.ui.tableArrivals->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(selectRow(const QModelIndex&, const QModelIndex&)));

	for ( int i = 0; i < ArrivalListColumns::Quantity; ++i ) {
		SC_D.ui.tableArrivals->setColumnHidden(i, !colVisibility[i]);
	}

	//SC_D.ui.tableArrivals->resize(SC_D.ui.tableArrivals->size());
	SC_D.ui.tableArrivals->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	SC_D.ui.buttonEditComment->setEnabled(SC_D.baseEvent.get());

	// Reset custom labels and set background
	resetCustomLabels();

	if ( SC_D.currentOrigin == nullptr ) {
		SC_D.ui.cbLocator->setEnabled(false);
		SC_D.ui.retranslateUi(this);
		SC_D.ui.btnShowWaveforms->setEnabled(false);
		SC_D.ui.btnRelocate->setEnabled(false);
		SC_D.ui.cbLocatorProfile->setEnabled(false);
		SC_D.ui.btnCustom0->setEnabled(false);
		SC_D.ui.btnCustom1->setEnabled(false);
		SC_D.map->setOrigin(nullptr);
		SC_D.residuals->update();
		return;
	}

	SC_D.ui.btnCustom0->setEnabled(true);
	SC_D.ui.btnCustom1->setEnabled(true);

	if ( SC_D.ui.cbLocator->count() > 1 ) {
		SC_D.ui.cbLocator->setEnabled(true);
	}

	SC_D.ui.btnLocatorSettings->setEnabled(SC_D.locator != nullptr);

	SC_D.ui.btnShowWaveforms->setEnabled(true);
	SC_D.ui.btnRelocate->setEnabled(true);
	SC_D.ui.cbLocatorProfile->setEnabled(true);
	SC_D.ui.btnCommit->setEnabled(true);

	//SC_D.ui.cbFixedDepth->setChecked(Qt::Unchecked);
	Time t = SC_D.currentOrigin->time();
	SC_D.ui.labelRegion->setText(Regions::getRegionName(SC_D.currentOrigin->latitude(), SC_D.currentOrigin->longitude()).c_str());
	//timeToLabel(SC_D.ui.labelDate, timeToString(t, "%Y-%m-%d");
	std::string format = "%Y-%m-%d %H:%M:%S";
	if ( SCScheme.precision.originTime > 0 ) {
		format += ".%";
		format += Core::toString(SCScheme.precision.originTime);
		format += "f";
	}

	timeToLabel(SC_D.ui.labelTime, t, format.c_str());

	double radius;
	if ( SC_D.config.defaultEventRadius > 0 ) {
		radius = SC_D.config.defaultEventRadius;
	}
	else {
		radius = 20;
		try {
			radius = std::min(radius, SC_D.currentOrigin->quality().maximumDistance()+0.1);
		}
		catch ( ... ) {}
	}

	if ( SC_D.undoList.isEmpty() ) {
		SC_D.map->canvas().displayRect(QRectF(SC_D.currentOrigin->longitude()-radius, SC_D.currentOrigin->latitude()-radius, radius*2, radius*2));
	}

	SC_D.map->canvas().setMapCenter(QPointF(SC_D.currentOrigin->longitude(), SC_D.currentOrigin->latitude()));
	//SC_D.map->setView(QPointF(SC_D.currentOrigin->longitude().value(), SC_D.currentOrigin->latitude().value()), SC_D.map->zoomLevel());
	SC_D.map->setOrigin(SC_D.currentOrigin.get());
	SC_D.map->update();
	if ( SC_D.toolMap ) {
		if ( SC_D.undoList.isEmpty() )
			SC_D.toolMap->canvas().displayRect(QRectF(SC_D.currentOrigin->longitude()-radius, SC_D.currentOrigin->latitude()-radius, radius*2, radius*2));

		SC_D.toolMap->canvas().setMapCenter(QPointF(SC_D.currentOrigin->longitude(), SC_D.currentOrigin->latitude()));
		//SC_D.toolMap->setView(QPointF(SC_D.currentOrigin->longitude().value(), SC_D.currentOrigin->latitude().value()), SC_D.toolMap->zoomLevel());
		SC_D.toolMap->setOrigin(SC_D.currentOrigin.get());
		SC_D.toolMap->update();
	}

	SC_D.ui.labelLatitude->setText(latitudeToString(SC_D.currentOrigin->latitude(), true, false, SCScheme.precision.location));
	SC_D.ui.labelLatitudeUnit->setText(latitudeToString(SC_D.currentOrigin->latitude(), false, true));
	//SC_D.ui.labelLatitudeUnit->setText("deg");
	try {
		SC_D.ui.labelLatitudeError->setText(QString("+/- %1").arg(quantityUncertainty(SC_D.currentOrigin->latitude()), 0, 'f', SCScheme.precision.uncertainties));
		SC_D.ui.labelLatitudeErrorUnit->setText("km");
	}
	catch ( ValueException& ) {
		SC_D.ui.labelLatitudeError->setText("");
		SC_D.ui.labelLatitudeErrorUnit->setText("");
	}

	SC_D.ui.labelLongitude->setText(longitudeToString(SC_D.currentOrigin->longitude(), true, false, SCScheme.precision.location));
	SC_D.ui.labelLongitudeUnit->setText(longitudeToString(SC_D.currentOrigin->longitude(), false, true, SCScheme.precision.location));
	//SC_D.ui.labelLongitudeUnit->setText("deg");
	try {
		SC_D.ui.labelLongitudeError->setText(QString("+/- %1").arg(quantityUncertainty(SC_D.currentOrigin->longitude()), 0, 'f', SCScheme.precision.uncertainties));
		SC_D.ui.labelLongitudeErrorUnit->setText("km");
	}
	catch ( ValueException& ) {
		SC_D.ui.labelLongitudeError->setText("");
		SC_D.ui.labelLongitudeErrorUnit->setText("");
	}

	try {
		SC_D.ui.labelDepth->setText(depthToString(SC_D.currentOrigin->depth(), SCScheme.precision.depth));
		SC_D.ui.editFixedDepth->setText(depthToString(SC_D.currentOrigin->depth(), std::max(3, SCScheme.precision.depth)));
		SC_D.ui.labelDepthUnit->setText("km");
	}
	catch ( ValueException& ) {
		SC_D.ui.labelDepth->setText("-");
		SC_D.ui.editFixedDepth->setText("");
	}

	try {
		SC_D.ui.labelDepth->setToolTip(tr("Type: %1").arg(SC_D.currentOrigin->depthType().toString()));
	}
	catch ( ValueException& ) {
		SC_D.ui.labelDepth->setToolTip(tr("Type: unset"));
	}

	try {
		double err_z = quantityUncertainty(SC_D.currentOrigin->depth());
		if ( err_z == 0.0 ) {
			SC_D.ui.labelDepthError->setText(QString("fixed"));
			SC_D.ui.labelDepthErrorUnit->setText("");

			//SC_D.ui.cbFixedDepth->setChecked(true);
		}
		else {
			SC_D.ui.labelDepthError->setText(QString("+/- %1").arg(err_z, 0, 'f', SCScheme.precision.uncertainties));
			SC_D.ui.labelDepthErrorUnit->setText("km");

			//SC_D.ui.cbFixedDepth->setChecked(false);
		}
	}
	catch ( ValueException& ) {
		SC_D.ui.labelDepthError->setText(QString("fixed"));
		SC_D.ui.labelDepthErrorUnit->setText("");
	}

	// When an origin has been loaded the depth is released
	SC_D.ui.cbFixedDepth->setChecked(false);

	try {
		SC_D.ui.labelStdError->setText(QString("%1").arg(SC_D.currentOrigin->quality().standardError(), 0, 'f', SCScheme.precision.rms));
	}
	catch ( ValueException& ) {
		SC_D.ui.labelStdError->setText("-");
	}

	SC_D.ui.labelComment->setText(SC_D.displayCommentDefault.c_str());
	if ( SC_D.displayComment ) {
		if ( SC_D.reader && SC_D.currentOrigin->commentCount() == 0 )
			SC_D.reader->loadComments(SC_D.currentOrigin.get());

		for ( size_t i = 0; i < SC_D.currentOrigin->commentCount(); ++i ) {
			if ( SC_D.currentOrigin->comment(i)->id() == SC_D.displayCommentID ) {
				SC_D.ui.labelComment->setText(SC_D.currentOrigin->comment(i)->text().c_str());
				break;
			}
		}
	}

	try {
		SC_D.ui.labelAzimuthGap->setText(QString("%1").arg(SC_D.currentOrigin->quality().azimuthalGap(), 0, 'f', 0));
		//SC_D.ui.labelAzimuthGapUnit->setText("deg");
	}
	catch ( ValueException& ) {
		SC_D.ui.labelAzimuthGap->setText("-");
	}

	try {
		if ( SCScheme.unit.distanceInKM )
			SC_D.ui.labelMinDist->setText(QString("%1").arg(Math::Geo::deg2km(SC_D.currentOrigin->quality().minimumDistance()), 0, 'f', SCScheme.precision.distance));
		else
			SC_D.ui.labelMinDist->setText(QString("%1").arg(SC_D.currentOrigin->quality().minimumDistance(), 0, 'f', 1));
		//SC_D.ui.labelMinDistUnit->setText("deg");
	}
	catch ( ValueException& ) {
		SC_D.ui.labelMinDist->setText("-");
	}

	try {
		try {
			timeToLabel(SC_D.ui.labelCreated, SC_D.currentOrigin->creationInfo().modificationTime(), "%Y-%m-%d %H:%M:%S");
			try {
				SC_D.ui.labelCreated->setToolTip(tr("Creation time: %1").arg(timeToString(SC_D.currentOrigin->creationInfo().creationTime(), "%Y-%m-%d %H:%M:%S")));
			}
			catch ( ... ) {}
		}
		catch ( ... ) {
			timeToLabel(SC_D.ui.labelCreated, SC_D.currentOrigin->creationInfo().creationTime(), "%Y-%m-%d %H:%M:%S");
			SC_D.ui.labelCreated->setToolTip(tr("That is actually the creation time"));
		}
	}
	catch ( ValueException& ) {
		SC_D.ui.labelCreated->setText("");
	}

	SC_D.ui.cbDepthType->setCurrentIndex(0);

	int activeArrivals = 0;
	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		Arrival* arrival = SC_D.currentOrigin->arrival(i);
		Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		QColor baseColor, pickColor;

		if ( i%2 ) {
			baseColor = Qt::gray;
		}
		else {
			baseColor = Qt::lightGray;
		}

		if ( pick ) {
			try {
				switch ( pick->evaluationMode() ) {
					case MANUAL:
						pickColor = SCScheme.colors.arrivals.manual;
						break;
					case AUTOMATIC:
						pickColor = SCScheme.colors.arrivals.automatic;
						break;
					default:
						pickColor = SCScheme.colors.arrivals.undefined;
						break;
				};
			}
			catch ( ... ) {
				pickColor = SCScheme.colors.arrivals.undefined;
			}
		}
		else
			pickColor = SCScheme.colors.arrivals.undefined;

		addArrival(i, arrival, pick, pickColor);

		SC_D.modelArrivals.setUseArrival(i, arrival);
		SC_D.residuals->setValueSelected(i, SC_D.modelArrivals.useArrival(i));

		QColor pickStateColor = pickColor;
		if ( !SC_D.modelArrivals.useArrival(i) ) {
			pickStateColor = SCScheme.colors.arrivals.disabled;
		}
		else {
			++activeArrivals;
		}

		SC_D.modelArrivals.setRowColor(i, pickStateColor);
	}

	if ( SC_D.baseEvent ) {
		SC_D.ui.labelEventID->setText(SC_D.baseEvent->publicID().c_str());
		SC_D.ui.labelEventID->setToolTip(SC_D.baseEvent->publicID().c_str());
	}
	else {
		SC_D.ui.labelEventID->setText("-");
		SC_D.ui.labelEventID->setToolTip("");
	}

	try {
		SC_D.ui.labelAgency->setText(SC_D.currentOrigin->creationInfo().agencyID().c_str());
		SC_D.ui.labelAgency->setToolTip(SC_D.currentOrigin->creationInfo().agencyID().c_str());
	}
	catch ( Core::ValueException & ) {
		SC_D.ui.labelAgency->setText("-");
		SC_D.ui.labelAgency->setToolTip("");
	}

	try {
		SC_D.ui.labelUser->setText(SC_D.currentOrigin->creationInfo().author().c_str());
		SC_D.ui.labelUser->setToolTip(SC_D.currentOrigin->creationInfo().author().c_str());
	}
	catch ( Core::ValueException & ) {
		SC_D.ui.labelUser->setText("-");
		SC_D.ui.labelUser->setToolTip("");
	}

	QPalette pal = SC_D.ui.labelEvaluation->palette();
	pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
	SC_D.ui.labelEvaluation->setPalette(pal);

	QString evalMode;
	try {
		evalMode = SC_D.currentOrigin->evaluationStatus().toString();
		if ( SC_D.currentOrigin->evaluationStatus() == REJECTED ) {
			QPalette pal = SC_D.ui.labelEvaluation->palette();
			pal.setColor(QPalette::WindowText, Qt::red);
			SC_D.ui.labelEvaluation->setPalette(pal);
		}
	}
	catch ( ... ) {
		evalMode = "-";
	}

	try {
		if ( SC_D.currentOrigin->evaluationMode() == AUTOMATIC ) {
			evalMode += " (A)";
		}
		else if ( SC_D.currentOrigin->evaluationMode() == MANUAL ) {
			evalMode += " (M)";
		}
		else {
			evalMode += " (-)";
		}
	}
	catch ( ... ) {
		evalMode += " (-)";
	}

	SC_D.ui.labelEvaluation->setText(evalMode);
	SC_D.ui.labelMethod->setText(SC_D.currentOrigin->methodID().c_str());
	SC_D.ui.labelEarthModel->setText(SC_D.currentOrigin->earthModelID().c_str());

	SC_D.ui.labelNumPhases->setText(QString("%1").arg(activeArrivals));
	SC_D.ui.labelNumPhasesError->setText(QString("%1").arg(SC_D.currentOrigin->arrivalCount()));

	SC_D.residuals->updateBoundingRect();

	plotTabChanged(SC_D.plotTab->currentIndex());

	//SC_D.ui.tableArrivals->resizeColumnsToContents();
	SC_D.ui.tableArrivals->resizeRowsToContents();
	SC_D.ui.tableArrivals->sortByColumn(SC_D.ui.tableArrivals->horizontalHeader()->sortIndicatorSection(),
	                                 SC_D.ui.tableArrivals->horizontalHeader()->sortIndicatorOrder());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addArrival(int idx, const Arrival *arrival,
                                   const Pick *pick, const QColor& c) {
	int id = SC_D.residuals->count();

	SC_D.residuals->addValue(QPointF());

	SC_D.residuals->setValueColor(id, PC_DISTANCE, c);
	SC_D.residuals->setValueColor(id, PC_RESIDUAL, c);
	SC_D.residuals->setValueColor(id, PC_TRAVELTIME, c);
	SC_D.residuals->setValueColor(id, PC_FMAZI, c);
	//SC_D.residuals->setValueColor(id, 3, c);

	double dist = -1;
	try {
		dist = arrival->distance();
		if ( SCScheme.distanceHypocentral ) {
			double edep = SC_D.pickerConfig.defaultDepth;
			double salt = 0;
			try {
				edep = SC_D.currentOrigin->depth().value();
			}
			catch ( ... ) {}
			auto sloc = Client::Inventory::Instance()->getSensorLocation(pick);
			if ( sloc ) {
				salt = elevation(sloc);
			}
			dist = hypocentralDistance(dist, edep, salt);
		}
	}
	catch ( ... ) {}

	char phase = Util::getShortPhaseName(arrival->phase().code());
	if ( phase == 'S' ) {
		SC_D.residuals->setValueSymbol(id, DiagramWidget::Rectangle);
	}
	else if ( phase == 'P' ) {
		SC_D.residuals->setValueSymbol(id, DiagramWidget::Circle);
	}
	else if ( phase == 'L' or phase == 'R' or phase == 'Q' or phase == 'G' ) {
		// surface wave phase names defined by IASPEI, NMSOP
		SC_D.residuals->setValueSymbol(id, DiagramWidget::Triangle);
	}
	else if ( phase == 'I' ) {
		// infrasound
		SC_D.residuals->setValueSymbol(id, DiagramWidget::TriangleUpsideDown);
	}
	else {
		SC_D.residuals->setValueSymbol(id, DiagramWidget::Diamond);
	}

	try {
		SC_D.residuals->setValueColor(id, PC_AZIMUTH, SCScheme.colors.arrivals.residuals.colorAt(arrival->timeResidual()));
	}
	catch ( Core::ValueException& ) {
		SC_D.residuals->setValueColor(id, PC_AZIMUTH, SCScheme.colors.arrivals.undefined);
		//SC_D.residuals->setValueColor(id, 3, SCScheme.colors.arrivals.undefined);
	}

	SC_D.residuals->setValueColor(id, PC_REDUCEDTRAVELTIME, c);

	try {
		if ( SCScheme.unit.distanceInKM ) {
			SC_D.residuals->setValue(id, PC_DISTANCE, Math::Geo::deg2km(dist));
		}
		else {
			SC_D.residuals->setValue(id, PC_DISTANCE, dist);
		}
	}
	catch ( ValueException& ) {
		SC_D.residuals->setValue(id, PC_DISTANCE, 0.0);
		SC_D.residuals->setValueValid(id, PC_DISTANCE, false);
	}

	SC_D.modelArrivals.setDistance(id, SC_D.residuals->value(id, PC_DISTANCE));

	try {
		double residual = arrival->timeResidual();
		double lowerUncertainty = -1, upperUncertainty = -1;

		if ( pick ) {
			try {
				lowerUncertainty = pick->time().lowerUncertainty();
				upperUncertainty = pick->time().upperUncertainty();
			}
			catch ( ... ) {
				try {
					lowerUncertainty = upperUncertainty = pick->time().uncertainty();
				}
				catch ( ... ) {
					lowerUncertainty = upperUncertainty = -1;
				}
			}
		}

		if ( upperUncertainty < 0 ) {
			SC_D.residuals->setValue(id, PC_RESIDUAL, residual);
		}
		else {
			SC_D.residuals->setValue(id, PC_RESIDUAL, residual, lowerUncertainty, upperUncertainty);
		}
	}
	catch ( ValueException& ) {
		SC_D.residuals->setValue(id, PC_RESIDUAL, 0.0);
		SC_D.residuals->setValueValid(id, PC_RESIDUAL, false);
	}

	if ( pick ) {
		SC_D.residuals->setValue(id, PC_TRAVELTIME, static_cast<double>(pick->time().value() - SC_D.currentOrigin->time().value()));
	}
	else {
		SC_D.residuals->setValue(id, PC_TRAVELTIME, 0.0);
		SC_D.residuals->setValueValid(id, PC_TRAVELTIME, false);
	}

	try {
		SC_D.residuals->setValue(id, PC_AZIMUTH, arrival->azimuth());
	}
	catch ( ValueException& ) {
		SC_D.residuals->setValue(id, PC_AZIMUTH, 0.0);
		SC_D.residuals->setValueValid(id, PC_AZIMUTH, false);
	}

	if ( SC_D.residuals->isValueValid(id, PC_DISTANCE) && SC_D.residuals->isValueValid(id, PC_TRAVELTIME) ) {
		if ( SCScheme.unit.distanceInKM ) {
			SC_D.residuals->setValue(id, PC_REDUCEDTRAVELTIME, SC_D.residuals->value(id,PC_TRAVELTIME) - SC_D.residuals->value(id,PC_DISTANCE)/SC_D.config.reductionVelocityP);
		}
		else {
			SC_D.residuals->setValue(id, PC_REDUCEDTRAVELTIME, SC_D.residuals->value(id,PC_TRAVELTIME) - Math::Geo::deg2km(SC_D.residuals->value(id,PC_DISTANCE))/SC_D.config.reductionVelocityP);
		}
	}
	else {
		SC_D.residuals->setValue(id, PC_REDUCEDTRAVELTIME, 0.0);
		SC_D.residuals->setValueValid(id, PC_REDUCEDTRAVELTIME, false);
	}

	if ( SC_D.residuals->isValueValid(id, PC_DISTANCE) &&
	     SC_D.residuals->isValueValid(id, PC_AZIMUTH) &&
	     phase == 'P' && SC_D.currentOrigin ) {

		PlotWidget::PolarityType polarity = PlotWidget::POL_UNSET;
		if ( !pick ) {
			pick = Pick::Find(arrival->pickID());
		}

		if ( pick ) {
			try {
				switch ( pick->polarity() ) {
					case POSITIVE:
						polarity = PlotWidget::POL_POSITIVE;
						break;
					case NEGATIVE:
						polarity = PlotWidget::POL_NEGATIVE;
						break;
					case UNDECIDABLE:
						polarity = PlotWidget::POL_UNDECIDABLE;
						break;
					default:
						break;
				}
			}
			catch ( ... ) {}

			if ( Util::getShortPhaseName(arrival->phase().code()) != 'P' ) {
				polarity = PlotWidget::POL_UNSET;
			}

			SC_D.residuals->setValue(id, PC_POLARITY, polarity);
		}

		double beta;
		bool hasTakeOff;
		try {
			beta = arrival->takeOffAngle();
			hasTakeOff = true;
		}
		catch ( ... ) {
			hasTakeOff = false;
		}

		if ( !hasTakeOff && SC_D.config.computeMissingTakeOffAngles ) {
			double lat, lon;
			double azi = SC_D.residuals->value(id, PC_AZIMUTH);

			Math::Geo::delandaz2coord(
				dist, azi,
				SC_D.currentOrigin->latitude(), SC_D.currentOrigin->longitude(),
				&lat, &lon
			);

			try {
				TravelTime ttt = SC_D.ttTable.computeFirst(
					SC_D.currentOrigin->latitude(), SC_D.currentOrigin->longitude(),
					SC_D.currentOrigin->depth(), lat, lon
				);

				beta = ttt.takeoff;
				SC_D.modelArrivals.setTakeOffAngle(idx, beta);

				hasTakeOff = true;
			}
			catch ( ... ) {}
		}

		if ( hasTakeOff && static_cast<PlotWidget*>(SC_D.residuals)->shape(polarity).shown ) {
			double azi;
			azi = SC_D.residuals->value(id, PC_AZIMUTH);

			if ( beta > 90 ) {
				beta = 180-beta;
				azi = azi-180;
				if ( azi < 0 ) azi += 360;
			}

			beta = sqrt(2.0) * sin(0.5*deg2rad(beta));

			//if ( static_cast<PlotWidget*>(SC_D.residuals)->shape(polarity).colorUsed )
			//	SC_D.residuals->setValueColor(id, PC_FMAZI, static_cast<PlotWidget*>(SC_D.residuals)->shape(polarity).color);

			SC_D.residuals->setValue(id, PC_FMAZI, azi);
			SC_D.residuals->setValue(id, PC_FMDIST, beta);
		}
		else {
			SC_D.residuals->setValue(id, PC_FMDIST, 0.0);
			SC_D.residuals->setValue(id, PC_FMAZI, 0.0);
			SC_D.residuals->setValueValid(id, PC_FMDIST, false);
			SC_D.residuals->setValueValid(id, PC_FMAZI, false);
		}
	}
	else {
		SC_D.residuals->setValue(id, PC_FMDIST, 0.0);
		SC_D.residuals->setValue(id, PC_FMAZI, 0.0);
		SC_D.residuals->setValueValid(id, PC_FMDIST, false);
		SC_D.residuals->setValueValid(id, PC_FMAZI, false);
	}

	if ( SC_D.plotFilter )
		SC_D.residuals->showValue(id, SC_D.plotFilter->accepts(SC_D.residuals, id));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addPick(Seiscomp::DataModel::Pick* pick) {
	if ( SC_D.recordView )
		SC_D.recordView->addPick(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setStationEnabled(const std::string& networkCode,
                                          const std::string& stationCode,
                                          bool state) {
	if ( SC_D.recordView )
		SC_D.recordView->setStationEnabled(networkCode, stationCode, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::importArrivals() {
	stopBlinking();

	if ( !SC_D.reader ) return;

	EventPtr event = SC_D.baseEvent;

	if ( !event ) {
		event = Event::Cast(SC_D.reader->getEvent(SC_D.baseOrigin->publicID()));
		if ( !event )
			event = Event::Cast(SC_D.reader->getEvent(SC_D.currentOrigin->publicID()));

		if ( !event ) {
			QMessageBox::critical(this, "ImportPicks::Error",
			                      "This location has not been associated with an event");
			return;
		}
	}

	ImportPicksDialog *dlg = new ImportPicksDialog(this);
	if ( dlg->exec() == QDialog::Rejected ) {
		delete dlg;
		return;
	}

	ImportPicksDialog::Selection sel = dlg->currentSelection();
	bool importAllPicks = dlg->importAllPicks();
	bool importAllPhases = dlg->importAllPhases();
	bool preferTargetPhases = dlg->preferTargetPhases();
	delete dlg;

	qApp->setOverrideCursor(Qt::WaitCursor);

	OriginPtr referenceOrigin;
	bool associateOnly = false;

	DataModel::PublicObjectTimeSpanBuffer cache(SC_D.reader, Core::TimeSpan(3600,0));
	typedef std::pair<std::string,int> PhaseWithFlags;
	typedef std::map<std::string, PhaseWithFlags> PhasePicks;

	PhasePicks sourcePhasePicks;

	switch ( sel ) {
		case ImportPicksDialog::LatestOrigin:
			{
				Core::Time maxTime;
				OriginPtr latestOrigin;
				DatabaseIterator or_it = SC_D.reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					++or_it;

					try {
						if ( o->creationInfo().creationTime() > maxTime ) {
							latestOrigin = o;
							maxTime = o->creationInfo().creationTime();
							SEISCOMP_DEBUG("MaxTime: %s, Origin: %s", Core::toString(maxTime).c_str(), o->publicID().c_str());
						}
					}
					catch ( ... ) { continue; }
				}
				or_it.close();

				if ( !latestOrigin || latestOrigin->publicID() == SC_D.currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is no later origin than the current");
					QMessageBox::information(this, "ImportPicks", "There is no later origin than the current.");
					qApp->restoreOverrideCursor();
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					SC_D.reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::LatestAutomaticOrigin:
			{
				Core::Time maxTime;
				OriginPtr latestOrigin;
				DatabaseIterator or_it = SC_D.reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					++or_it;

					// try {
					// 	if ( o->status() != AUTOMATIC_ORIGIN ) { continue; }
					// }
					// catch ( ... ) {}

					try {
						if ( o->evaluationMode() != AUTOMATIC )
							continue;
					}
					catch ( ... ) {}

					try {
						if ( o->creationInfo().creationTime() > maxTime ) {
							latestOrigin = o;
							maxTime = o->creationInfo().creationTime();
							SEISCOMP_DEBUG("MaxTime: %s, Origin: %s", Core::toString(maxTime).c_str(), o->publicID().c_str());
						}
					}
					catch ( ... ) { continue; }
				}
				or_it.close();

				if ( !latestOrigin || latestOrigin->publicID() == SC_D.currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is no later origin than the current");
					QMessageBox::information(this, "ImportPicks", "There is no later origin than the current.");
					qApp->restoreOverrideCursor();
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					SC_D.reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::MaxPhaseOrigin:
			{
				size_t maxPhase = 0;
				OriginPtr latestOrigin;
				std::vector<OriginPtr> origins;
				DatabaseIterator or_it = SC_D.reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					if ( o ) origins.push_back(o);
					++or_it;
				}
				or_it.close();

				for ( size_t i = 0; i < origins.size(); ++i ) {
					OriginPtr o = origins[i];
					if ( o->arrivalCount() == 0 )
						SC_D.reader->loadArrivals(o.get());

					if ( o->arrivalCount() > maxPhase ) {
						latestOrigin = o;
						maxPhase = o->arrivalCount();
						SEISCOMP_DEBUG("MaxPhase: %lu, Origin: %s", (unsigned long)maxPhase, o->publicID().c_str());
					}
				}

				if ( !latestOrigin || latestOrigin->publicID() == SC_D.currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is origin with more phases than the current");
					qApp->restoreOverrideCursor();
					QMessageBox::information(this, "ImportPicks", "There is no origin with more phases than the current.");
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					SC_D.reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::AllOrigins:
		{
			std::vector<OriginPtr> origins;
			DatabaseIterator or_it = SC_D.reader->getOrigins(event->publicID());
			while ( *or_it ) {
				OriginPtr o = Origin::Cast(*or_it);
				if ( o ) origins.push_back(o);
				++or_it;
			}
			or_it.close();

			for ( size_t i = 0; i < origins.size(); ++i ) {
				OriginPtr o = origins[i];
				if ( o->arrivalCount() == 0 )
					SC_D.reader->loadArrivals(o.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
					Arrival *ar = o->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}

			associateOnly = true;
			break;
		}
	};

	PickedPhases sourcePhases, targetPhases, *sourcePhasesPtr, *targetPhasesPtr;

	// Collect source phases grouped by stream
	for ( PhasePicks::iterator it = sourcePhasePicks.begin(); it != sourcePhasePicks.end(); ++it ) {
		PickPtr pick = cache.get<Pick>(it->first);
		if ( !pick ) {
			SEISCOMP_WARNING("Pick %s not found: ignoring", it->first.c_str());
			continue;
		}

		// Filter agency
		if ( !importAllPicks && (objectAgencyID(pick.get()) != SCApp->agencyID()) )
			continue;

		char phaseCode[2] = {'\0', '\0'};
		try { phaseCode[0] = Util::getShortPhaseName(it->second.first); }
		catch ( ... ) {}

		if ( phaseCode[0] == '\0' )
			phaseCode[0] = 'P';

		if ( !importAllPhases )
			sourcePhases[PickPhase(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode(), phaseCode)] = PickWithFlags(pick, it->second.second);
		else
			sourcePhases[PickPhase(wfid2str(pick->waveformID()), it->second.first)] = PickWithFlags(pick, it->second.second);
	}

	// Collect target phases grouped by stream
	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		Arrival *ar = SC_D.currentOrigin->arrival(i);
		PickPtr p = cache.get<Pick>(ar->pickID());

		if ( !p ) continue;

		WaveformStreamID &wfsi = p->waveformID();

		char phaseCode[2] = {'\0', '\0'};
		try { phaseCode[0] = Util::getShortPhaseName(ar->phase().code()); }
		catch ( ... ) {}

		if ( phaseCode[0] == '\0' )
			phaseCode[0] = 'P';

		int flags = Seismology::arrivalToFlags(ar);

		if ( !importAllPhases )
			targetPhases[PickPhase(wfsi.networkCode() + "." + wfsi.stationCode(), phaseCode)] = PickWithFlags(p, flags);
		else
			targetPhases[PickPhase(wfid2str(wfsi), ar->phase().code())] = PickWithFlags(p, flags);
	}

	sourcePhasesPtr = &sourcePhases;
	targetPhasesPtr = &targetPhases;

	if ( !preferTargetPhases )
		std::swap(sourcePhasesPtr, targetPhasesPtr);

	qApp->restoreOverrideCursor();

	if ( !merge(sourcePhasesPtr, targetPhasesPtr, true, associateOnly, preferTargetPhases) ) {
		SEISCOMP_DEBUG("No additional picks to merge");
		QMessageBox::information(this, "ImportPicks", "There are no additional "
		                         "streams with picks to merge.");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::merge(void *sourcePhases, void *targetPhases,
                              bool checkDuplicates, bool associateOnly,
                              bool failOnNoNewPhases) {
	PickedPhases *sourcePhasesPtr, *targetPhasesPtr;
	set<string> usedPickIDs;

	std::vector<PhasePickWithFlags> additionalPicks;
	typedef std::pair<std::string, std::string> PhaseStream;
	typedef std::map<PhaseStream, PickWithFlags> NewPhases;

	NewPhases newPhases;

	sourcePhasesPtr = reinterpret_cast<PickedPhases*>(sourcePhases);
	targetPhasesPtr = reinterpret_cast<PickedPhases*>(targetPhases);

	for ( PickedPhases::iterator it = targetPhasesPtr->begin(); it != targetPhasesPtr->end(); ++it ) {
		PickPtr p = it->second.first;
		usedPickIDs.insert(p->publicID());
	}

	// Merge source phases with target phases
	for ( PickedPhases::iterator it = sourcePhasesPtr->begin(); it != sourcePhasesPtr->end(); ++it ) {
		PickPtr p = it->second.first;

		// Do we have the same phase already in the target?
		if ( checkDuplicates && targetPhasesPtr->find(it->first) != targetPhasesPtr->end() ) {
			SEISCOMP_INFO("- phase %s for stream %s (already in target)",
			              it->first.second.c_str(), it->first.first.c_str());
			continue;
		}

		// Final check, if the same pick id is already or will be associated
		// with current origin
		if ( usedPickIDs.find(p->publicID()) != usedPickIDs.end() ) {
			SEISCOMP_INFO("- pick %s as phase %s for stream %s (pick already in target)",
			              p->publicID().c_str(), it->first.second.c_str(),
			              it->first.first.c_str());
			continue;
		}

		usedPickIDs.insert(p->publicID());

		SEISCOMP_INFO("+ pick %s as phase %s for stream %s",
		              p->publicID().c_str(), it->first.second.c_str(),
		              it->first.first.c_str());

		PhaseStream ps(it->first);
		PickWithFlags newPick = newPhases[ps];

		if ( newPick.first ) {
			try {
				// Pick is older than the already inserted one: skip it
				if ( p->creationInfo().creationTime() < newPick.first->creationInfo().creationTime() )
					continue;
			}
			catch ( ... ) {
				// No creationTime set: take the first one
				continue;
			}
		}

		newPhases[ps] = PickWithFlags(p, it->second.second);
	}

	if ( failOnNoNewPhases && newPhases.empty() ) return false;

	SEISCOMP_DEBUG("*** Prepare merged origin ***");
	OriginPtr org = Origin::Create();
	org->assign(SC_D.currentOrigin.get());
	for ( PickedPhases::iterator it = targetPhasesPtr->begin(); it != targetPhasesPtr->end(); ++it ) {
		ArrivalPtr arrival = new Arrival;
		arrival->setPickID(it->second.first->publicID());
		arrival->setWeight(it->second.second ? 0 : 1);
		Seismology::flagsToArrival(arrival.get(), it->second.second);
		arrival->setPhase(Phase(it->first.second));
		org->add(arrival.get());
		SEISCOMP_DEBUG("! pick %s as phase %s for stream %s with flags %d",
		               it->second.first->publicID().c_str(), it->first.second.c_str(),
		               it->first.first.c_str(), it->second.second);
	}

	for ( NewPhases::iterator it = newPhases.begin(); it != newPhases.end(); ++it ) {
		PhasePickWithFlags ppwf;
		ppwf.pick = it->second.first;
		ppwf.phase = it->first.second;
		ppwf.flags = it->second.second;
		additionalPicks.push_back(ppwf);
		SEISCOMP_DEBUG("A pick %s as phase %s for stream %s with flags %d",
		               it->second.first->publicID().c_str(), it->first.second.c_str(),
		               wfid2str(it->second.first->waveformID()).c_str(),
		               it->second.second);
	}

	if ( org->arrivalCount() == 0 ) {
		for ( size_t i = 0; i < additionalPicks.size(); ++i ) {
			SensorLocation *sloc = SC_D.locator->getSensorLocation(Pick::Find(additionalPicks[i].pick->publicID()));
			if ( sloc == nullptr ) continue;

			ArrivalPtr arrival = new Arrival();
			arrival->setPickID(additionalPicks[i].pick->publicID());
			Seismology::flagsToArrival(arrival.get(), 0);
			arrival->setWeight(0.0);

			double az, baz, dist;
			Math::Geo::delazi(org->latitude().value(), org->longitude().value(),
			                  sloc->latitude(), sloc->longitude(), &dist, &az, &baz);

			arrival->setDistance(dist);
			arrival->setAzimuth(az);

			try {
				if ( additionalPicks[i].phase != "" )
					arrival->setPhase(additionalPicks[i].phase);
				else
					arrival->setPhase(Phase("P"));
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}

			double depth = 10.0, elev = 0.0;
			try { depth = org->depth().value(); }
			catch ( ... ) {}
			try { elev = sloc->elevation(); }
			catch ( ... ) {}

			try {
				double ttime = SC_D.ttTable.computeTime(arrival->phase().code().c_str(),
				                      org->latitude().value(), org->longitude().value(), depth,
				                      sloc->latitude(), sloc->longitude(), elev);

				double at = (double)(additionalPicks[i].pick->time().value()-org->time().value());
				arrival->setTimeResidual(at-ttime);
			}
			catch ( ... ) {}

			org->add(arrival.get());
		}

		applyNewOrigin(org.get(), true);
	}
	else
		relocate(org.get(), &additionalPicks, associateOnly, false, false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::showWaveforms() {
	if ( SC_D.recordView ) {
		SC_D.recordView->activateWindow();
		SC_D.recordView->raise();
		return;
	}

	if ( !SC_D.currentOrigin ) return;

	SC_D.recordView = new PickerView(nullptr, Qt::Window);
	SC_D.recordView->setDatabase(SC_D.reader);
	connect(SC_D.recordView, SIGNAL(requestArtificialOrigin(double, double, double, Seiscomp::Core::Time)),
	        this, SLOT(artificialOriginRequested(double, double, double, Seiscomp::Core::Time)));

	try {
		SC_D.recordView->setBroadBandCodes(SCApp->configGetStrings("picker.velocityChannelCodes"));
	}
	catch ( ... ) {}

	try {
		SC_D.recordView->setStrongMotionCodes(SCApp->configGetStrings("picker.accelerationChannelCodes"));
	}
	catch ( ... ) {}

	try {
		auto patterns = SCApp->configGetStrings("picker.auxiliary.channels");
		double minDist = 0, maxDist = 1000;
		try {
			minDist = SCApp->configGetDouble("picker.auxiliary.minimumDistance");
		}
		catch ( ... ) {}
		try {
			maxDist = SCApp->configGetDouble("picker.auxiliary.maximumDistance");
		}
		catch ( ... ) {}
		SC_D.recordView->setAuxiliaryChannels(patterns, minDist, maxDist);
	}
	catch ( ... ) {}

	QString errorMsg;
	if ( !SC_D.recordView->setConfig(SC_D.pickerConfig, &errorMsg) ) {
		QMessageBox::information(this, "Picker Error", errorMsg);
		delete SC_D.recordView;
		SC_D.recordView = nullptr;
		return;
	}

	/*
	for ( QVector<QPair<QString,QString> >::const_iterator it = _filters.begin();
	      it != _filters.end(); ++it ) {
		SC_D.recordView->addFilter(it->first, it->second);
	}

	SC_D.recordView->activateFilter(0);

	if ( !SC_D.recordView->setDataSource(_streamURL.c_str()) ) {
		QMessageBox::information(this, "RecordStream Error",
		                         QString("Setting recordstream '%1' failed.")
		                           .arg(_streamURL.c_str()));
		delete SC_D.recordView;
		SC_D.recordView = nullptr;
		return;
	}
	*/

	SC_D.recordView->setAttribute(Qt::WA_DeleteOnClose);

	setPickerView(SC_D.recordView);

	SC_D.recordView->setOrigin(SC_D.currentOrigin.get(), -5*60, 30*60);

	QVector<int> selectedArrivals = SC_D.residuals->getSelectedValues();
	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i )
		SC_D.recordView->setArrivalState(i, SC_D.residuals->isValueSelected(i));

	SC_D.recordView->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate() {
	relocate(SC_D.currentOrigin.get(), nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate(std::vector<PhasePickWithFlags>* additionalPicks,
                                 bool associateOnly, bool replaceExistingPhases) {
	relocate(SC_D.currentOrigin.get(), additionalPicks, associateOnly, replaceExistingPhases);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate(DataModel::Origin *org,
                                 std::vector<PhasePickWithFlags>* additionalPicks,
                                 bool associateOnly, bool replaceExistingPhases,
                                 bool useArrivalTable) {
	OriginPtr oldOrigin;
	OriginPtr origin;

	if ( !SC_D.locator ) {
		QMessageBox::critical(this, "Locator error", "No locator set.");
		return;
	}

	if ( SC_D.modelArrivals.useNoArrivals() ) {
		QMessageBox::critical(this, "Relocation error", "A relocation cannot be done without any arrivals.");
		return;
	}

	SC_D.locator->setProfile(SC_D.ui.cbLocatorProfile->currentText().toStdString());

	oldOrigin = Origin::Create();
	oldOrigin->assign(org);

	if ( useArrivalTable ) {
		for ( int i = 0; i < SC_D.modelArrivals.rowCount(); ++i ) {
			if ( !SC_D.modelArrivals.isRowEnabled(i) ) {
				continue;
			}

			ArrivalPtr arrival = new Arrival(*org->arrival(i));
			arrival->setBackazimuthUsed(SC_D.modelArrivals.backazimuthUsed(i));
			arrival->setTimeUsed(SC_D.modelArrivals.timeUsed(i));
			arrival->setHorizontalSlownessUsed(SC_D.modelArrivals.horizontalSlownessUsed(i));

			if ( arrival->timeUsed() || arrival->backazimuthUsed() || arrival->horizontalSlownessUsed() ) {
				arrival->setWeight(1.0);
			}
			else {
				arrival->setWeight(0.0);
			}

			if ( !SC_D.locator->getSensorLocation(Pick::Find(arrival->pickID())) ) {
				continue;
			}

			oldOrigin->add(arrival.get());

			try {
				if ( arrival->phase().code() == "" ) {
					arrival->setPhase(Phase("P"));
				}
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}
		}
	}
	else {
		for ( size_t i = 0; i < org->arrivalCount(); ++i ) {
			ArrivalPtr ar = org->arrival(i);

			try {
				if ( !SC_D.locator->getSensorLocation(Pick::Find(ar->pickID())) &&
					 ar->weight() < 0.5 )
					continue;
			}
			catch ( ... ) {}

			ArrivalPtr ar2 = new Arrival(*ar);

			oldOrigin->add(ar2.get());
		}
	}

	if ( replaceExistingPhases ) {
		while ( oldOrigin->arrivalCount() > 0 ) {
			oldOrigin->removeArrival(0);
		}
	}

	if ( additionalPicks ) {
		for ( size_t i = 0; i < additionalPicks->size(); ++i ) {
			SensorLocation *sloc = SC_D.locator->getSensorLocation(Pick::Find((*additionalPicks)[i].pick->publicID()));
			if ( sloc == nullptr ) continue;

			ArrivalPtr arrival = new Arrival();
			arrival->setPickID((*additionalPicks)[i].pick->publicID());

			if ( associateOnly ) {
				Seismology::flagsToArrival(arrival.get(), 0);
				arrival->setWeight(0.0);
			}
			else {
				Seismology::flagsToArrival(arrival.get(), (*additionalPicks)[i].flags);
				arrival->setWeight(1.0);
			}

			try {
				if ( (*additionalPicks)[i].phase != "" )
					arrival->setPhase((*additionalPicks)[i].phase);
				else
					arrival->setPhase(Phase("P"));
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}

			oldOrigin->add(arrival.get());
		}
	}

	bool fixedDepth = SC_D.ui.cbFixedDepth->isEnabled() && SC_D.ui.cbFixedDepth->isChecked();
	bool distanceCutOff = SC_D.ui.cbDistanceCutOff->isEnabled() && SC_D.ui.cbDistanceCutOff->isChecked();
	bool ignoreInitialLocation = SC_D.ui.cbIgnoreInitialLocation->isEnabled() && SC_D.ui.cbIgnoreInitialLocation->isChecked();

	if ( distanceCutOff )
		SC_D.locator->setDistanceCutOff(SC_D.ui.editDistanceCutOff->text().toDouble());
	else
		SC_D.locator->releaseDistanceCutOff();

	SC_D.locator->setIgnoreInitialLocation(ignoreInitialLocation);

	setCursor(Qt::WaitCursor);

	for ( int loop = 1; loop <= 2; ++loop ) {
		if ( fixedDepth ) {
			double depth = loop == 1 ? SC_D.ui.editFixedDepth->text().toDouble() : SC_D.minimumDepth;

			SC_D.locator->setFixedDepth(depth);

			SEISCOMP_DEBUG("setting depth to %.2f km", depth);
		}
		else
			SC_D.locator->releaseDepth();

		try {
			origin = Gui::relocate(SC_D.locator.get(), oldOrigin.get());
			/* DEBUG: Just copy the origin without relocating
			origin = Origin::Cast(oldOrigin->clone());
			origin->assign(oldOrigin.get());
			for ( size_t i = 0; i < oldOrigin->arrivalCount(); ++i ) {
				origin->add(Arrival::Cast(oldOrigin->arrival(i)->clone()));
			}
			*/

			if ( !origin ) {
				QMessageBox::critical(this, "Relocation error", "The relocation failed for some reason.");
				unsetCursor();
				return;
			}

			string msgWarning = SC_D.locator->lastMessage(Seismology::LocatorInterface::Warning);
			if ( !msgWarning.empty() ) {
				QMessageBox::warning(this, "Relocation warning", msgWarning.c_str());
			}
		}
		catch ( Core::GeneralException& e ) {
			// If relocation is enabled and fails retry it and just
			// associate
			if ( !associateOnly && additionalPicks ) {
				QMessageBox::critical(this, "Relocation error",
				       QString("Relocating failed. The new picks are going to be inserted with zero weight.\n%1").arg(e.what()));
				relocate(org, additionalPicks, true, replaceExistingPhases, useArrivalTable);
			}
			else
				QMessageBox::critical(this, "Relocation error", e.what());

			unsetCursor();
			return;
		}

		if ( !fixedDepth && SC_D.locator->supports(Seismology::LocatorInterface::FixedDepth) ) {
			try {
				if ( origin->depth() < SC_D.minimumDepth )
					origin->setDepth(RealQuantity(SC_D.minimumDepth,0.0,Core::None,Core::None,Core::None));
				else break;
			}
			catch ( ... ) {
				origin->setDepth(RealQuantity(SC_D.minimumDepth,0.0,Core::None,Core::None,Core::None));
			}
			oldOrigin = origin;
			fixedDepth = true;
		}
		else
			break;
	}

	unsetCursor();

	if ( fixedDepth ) {
		SC_D.ui.cbFixedDepth->setChecked(true);

		if ( SC_D.ui.cbDepthType->currentIndex() > 0 ) {
			if ( SC_D.ui.cbDepthType->currentIndex() > 1 ) {
				OriginDepthType type;
				type.fromInt(SC_D.ui.cbDepthType->itemData(SC_D.ui.cbDepthType->currentIndex()).toInt());
				origin->setDepthType(type);
			}
		}
		else
			origin->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
	}

	if ( distanceCutOff )
		SC_D.ui.cbDistanceCutOff->setChecked(true);

	if ( ignoreInitialLocation )
		SC_D.ui.cbIgnoreInitialLocation->setChecked(true);

	applyNewOrigin(origin.get(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::applyNewOrigin(DataModel::Origin *origin, bool relocated) {
	SEISCOMP_DEBUG("Created new origin %s", origin->publicID().c_str());

	origin->setEvaluationMode(EvaluationMode(MANUAL));
	origin->setEvaluationStatus(EvaluationStatus(CONFIRMED));
	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::UTC());
	origin->setCreationInfo(ci);

	pushUndo();

	SC_D.localOrigin = true;
	std::vector<DataModel::PickPtr> originPicks;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Pick* p = Pick::Find(origin->arrival(i)->pickID());
		if ( p ) {
			originPicks.push_back(p);
			SC_D.associatedPicks[p->publicID()] = p;
		}
	}

	SC_D.originPicks = originPicks;

	stopBlinking();

	SC_D.blockReadPicks = true;
	updateOrigin(origin);
	SC_D.blockReadPicks = false;

	//computeMagnitudes();
	SC_D.ui.btnMagnitudes->setEnabled(true);

	if ( SC_D.recordView ) {
		SC_D.recordView->setOrigin(origin);

		for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
			QModelIndex idx = SC_D.modelArrivals.index(i, USED);
			SC_D.recordView->setArrivalState(i, SC_D.modelArrivals.data(idx, UsedRole).toInt());
		}
	}

	emit newOriginSet(origin, SC_D.baseEvent.get(), SC_D.localOrigin, relocated);

	SC_D.ui.btnCommit->setText("Commit");
//	SC_D.ui.btnCommit->setMenu(SC_D.baseEvent?_commitMenu:nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::mergeOrigins(QList<DataModel::Origin*> orgs) {
	DataModel::PublicObjectTimeSpanBuffer cache(SC_D.reader, Core::TimeSpan(3600,0));
	typedef std::pair<std::string,double> PhaseWithWeight;
	typedef std::map<std::string, PhaseWithWeight> PhasePicks;

	PhasePicks sourcePhasePicks;

	Ui::MergeOrigins ui;
	QDialog dlg;
	ui.setupUi(&dlg);

	ui.labelInfo->setText(ui.labelInfo->text().arg(orgs.size()));

	if ( dlg.exec() != QDialog::Accepted ) return;

	qApp->setOverrideCursor(Qt::WaitCursor);

	bool importAllPicks = ui.checkAllAgencies->isChecked();

	// First origin is always the target of the drag and drop operation

	// Fill picks
	for ( int i = 0; i < orgs.size(); ++i ) {
		OriginPtr o = orgs[i];
		if ( o->arrivalCount() == 0 )
			SC_D.reader->loadArrivals(o.get());

		// Collect all picks with phases
		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			pair<PhasePicks::iterator,bool> itp;
			Arrival *ar = o->arrival(i);
			itp = sourcePhasePicks.insert(PhasePicks::value_type(ar->pickID(), PhaseWithWeight()));
			// Pick already exists
			if ( itp.second == false ) {
				SEISCOMP_DEBUG("Ignoring pick %s from Origin %s: pick already added to merge list",
				               ar->pickID().c_str(), o->publicID().c_str());
				continue;
			}

			double weight = 1.0; try { weight = ar->weight(); } catch ( ... ) {}
			try { itp.first->second = PhaseWithWeight(ar->phase().code(), weight); }
			catch ( ... ) { itp.first->second = PhaseWithWeight("P", weight); }
		}
	}

	PickedPhases sourcePhases, targetPhases;

	// Collect source phases grouped by stream
	for ( PhasePicks::iterator it = sourcePhasePicks.begin(); it != sourcePhasePicks.end(); ++it ) {
		PickPtr pick = cache.get<Pick>(it->first);
		if ( !pick ) {
			SEISCOMP_WARNING("Pick %s not found: ignoring", it->first.c_str());
			continue;
		}

		// Filter agency
		if ( !importAllPicks && (objectAgencyID(pick.get()) != SCApp->agencyID()) )
			continue;

		sourcePhases[PickPhase(wfid2str(pick->waveformID()), it->second.first)] = PickWithFlags(pick, it->second.second);
	}

	qApp->restoreOverrideCursor();

	Origin *oldCurrent = SC_D.currentOrigin.get();

	merge(&sourcePhases, &targetPhases, true, false, false);

	if ( oldCurrent != SC_D.currentOrigin ) emit locatorRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setLocalAmplitudes(Seiscomp::DataModel::Origin *org,
                                           AmplitudeSet *amps, StringSet *ampIDs) {
	if ( org != SC_D.currentOrigin ) return;

	for ( AmplitudeSet::iterator it = SC_D.changedAmplitudes.begin();
	      it != SC_D.changedAmplitudes.end(); ++it ) {
		if ( ampIDs->find(it->first->publicID()) != ampIDs->end() )
			amps->insert(*it);
	}

	// Store new amplitudes in current set
	SC_D.changedAmplitudes.swap(*amps);
	emit requestRaise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::computeMagnitudes() {
	emit computeMagnitudesRequested();

	SC_D.ui.btnMagnitudes->setEnabled(SC_D.currentOrigin->magnitudeCount() == 0);

	if ( SC_D.currentOrigin->magnitudeCount() > 0 ) {
		emit magnitudesAdded(SC_D.currentOrigin.get(), SC_D.baseEvent.get());
		evaluateOrigin(SC_D.currentOrigin.get(), SC_D.baseEvent.get(),
		               SC_D.localOrigin, false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::magnitudeRemoved(const QString &id, Seiscomp::DataModel::Object *obj) {
	if ( id != SC_D.currentOrigin->publicID().c_str() ) return;

	SC_D.ui.btnMagnitudes->setEnabled(SC_D.currentOrigin->magnitudeCount() == 0);

	if ( SC_D.currentOrigin->magnitudeCount() > 0 ) {
		evaluateOrigin(SC_D.currentOrigin.get(), SC_D.baseEvent.get(),
		               SC_D.localOrigin, false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::magnitudeSelected(const QString &id, Seiscomp::DataModel::Magnitude *mag) {
	if ( mag ) {
		SC_D.actionCommitOptions->setProperty("EvPrefMagType", QString(mag->type().c_str()));
	}
	else {
		SC_D.actionCommitOptions->setProperty("EvPrefMagType", QVariant());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::pushUndo() {
	SC_D.undoList.push(
		OriginLocatorViewPrivate::OriginMemento(
			SC_D.currentOrigin.get(), SC_D.changedPicks,
			SC_D.changedAmplitudes, SC_D.localOrigin
		)
	);

	if ( SC_D.undoList.size() > 20 )
		SC_D.undoList.pop();

	SC_D.redoList.clear();

	emit undoStateChanged(!SC_D.undoList.isEmpty());
	emit redoStateChanged(!SC_D.redoList.isEmpty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::undo() {
	if ( SC_D.undoList.isEmpty() ) {
		return false;
	}

	SC_D.redoList.push(
		OriginLocatorViewPrivate::OriginMemento(
			SC_D.currentOrigin.get(), SC_D.changedPicks,
			SC_D.changedAmplitudes, SC_D.localOrigin
		)
	);

	OriginPtr origin = SC_D.undoList.top().origin;
	SC_D.localOrigin = SC_D.undoList.top().newOrigin;
	SC_D.changedPicks = SC_D.undoList.top().newPicks;
	SC_D.changedAmplitudes = SC_D.undoList.top().newAmplitudes;
	SC_D.undoList.pop();

	SC_D.ui.btnMagnitudes->setEnabled(SC_D.localOrigin && (origin->magnitudeCount() == 0));

	SC_D.blockReadPicks = true;
	updateOrigin(origin.get());
	SC_D.blockReadPicks = false;

	if ( SC_D.recordView ) {
		SC_D.recordView->setOrigin(origin.get());

		for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
			QModelIndex idx = SC_D.modelArrivals.index(i, USED);
			SC_D.recordView->setArrivalState(i, SC_D.modelArrivals.data(idx, UsedRole).toInt());
		}
	}

	emit newOriginSet(origin.get(), SC_D.baseEvent.get(), SC_D.localOrigin, false);
	emit undoStateChanged(!SC_D.undoList.isEmpty());
	emit redoStateChanged(!SC_D.redoList.isEmpty());

	stopBlinking();

	return !SC_D.undoList.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::redo() {
	if ( SC_D.redoList.isEmpty() ) return false;

	SC_D.undoList.push(
		OriginLocatorViewPrivate::OriginMemento(
			SC_D.currentOrigin.get(), SC_D.changedPicks,
			SC_D.changedAmplitudes, SC_D.localOrigin
		)
	);

	OriginPtr origin = SC_D.redoList.top().origin;
	SC_D.localOrigin = SC_D.redoList.top().newOrigin;
	SC_D.changedPicks = SC_D.redoList.top().newPicks;
	SC_D.changedAmplitudes = SC_D.redoList.top().newAmplitudes;
	SC_D.redoList.pop();

	SC_D.ui.btnMagnitudes->setEnabled(SC_D.localOrigin && (origin->magnitudeCount() == 0));

	SC_D.blockReadPicks = true;
	updateOrigin(origin.get());
	SC_D.blockReadPicks = false;

	if ( SC_D.recordView ) {
		SC_D.recordView->setOrigin(origin.get());

		for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
			QModelIndex idx = SC_D.modelArrivals.index(i, USED);
			SC_D.recordView->setArrivalState(i, SC_D.modelArrivals.data(idx, UsedRole).toInt());
		}
	}

	emit newOriginSet(origin.get(), SC_D.baseEvent.get(), SC_D.localOrigin, false);
	emit undoStateChanged(!SC_D.undoList.isEmpty());
	emit redoStateChanged(!SC_D.redoList.isEmpty());

	stopBlinking();

	return !SC_D.redoList.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::createArtificialOrigin() {
	createArtificialOrigin(SC_D.map->canvas().mapCenter(), QPoint());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::createArtificialOrigin(const QPointF &epicenter,
                                              const QPoint &dialogPos) {
	createArtificialOrigin(epicenter, SC_D.pickerConfig.defaultDepth,
	                       Core::Time::UTC(), dialogPos);
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::createArtificialOrigin(const QPointF &epicenter,
                                               double depth,
                                               Seiscomp::Core::Time time,
                                               const QPoint &dialogPos) {
	OriginDialog dialog(this);
	try {
		if ( SCApp->configGetBool("olv.artificialOriginAdvanced") ) {
			dialog.enableAdvancedOptions();

			// build list of available magnitude types
			QStringList magList;
			Processing::MagnitudeProcessorFactory::ServiceNames *names =
			        Processing::MagnitudeProcessorFactory::Services();
			for ( Processing::MagnitudeProcessorFactory::ServiceNames::const_iterator it = names->begin();
			      it != names->end(); ++it) {
				magList << it->c_str();
			}
			dialog.setMagTypes(magList);
		}
	} catch ( Seiscomp::Config::Exception& ) {}

	dialog.loadSettings();
	dialog.setLongitude(epicenter.x());
	dialog.setLatitude(epicenter.y());
	dialog.setDepth(depth);
	dialog.setTime(time);

	if ( !dialogPos.isNull() )
		dialog.move(dialogPos.x(), dialogPos.y());

	if ( dialog.exec() == QDialog::Accepted ) {
		dialog.saveSettings();
		OriginPtr origin = Origin::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::UTC());
		origin->setCreationInfo(ci);
		origin->setLongitude(dialog.longitude());
		origin->setLatitude(dialog.latitude());
		origin->setDepth(RealQuantity(dialog.depth()));
		origin->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
		origin->setTime(Time(dialog.getTime_t(), 0));
		origin->setEvaluationMode(EvaluationMode(MANUAL));

		if ( dialog.advanced() ) {
			// magnitude
			std::string type = dialog.magType().toStdString();
			std::string id = origin->publicID() + "#netMag." + type;
			MagnitudePtr mag = Magnitude::Create(id);
			mag->setCreationInfo(ci);
			mag->setMagnitude(RealQuantity(dialog.magValue()));
			mag->setType(type);
			mag->setOriginID(origin->publicID());
			mag->setStationCount(dialog.phaseCount());
			origin->add(mag.get());

			// origin quality
			OriginQuality quality;
			quality.setUsedPhaseCount(dialog.phaseCount());
			origin->setQuality(quality);
		}

		emit artificalOriginCreated(origin.get());
	}
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setScript0(const std::string &script) {
	SC_D.script0 = script;
	SC_D.ui.btnCustom0->setVisible(!SC_D.script0.empty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setScript1(const std::string &script) {
	SC_D.script1 = script;
	SC_D.ui.btnCustom1->setVisible(!SC_D.script1.empty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::editComment() {
	if ( !SC_D.baseEvent ) return;

	CommentEdit dlg;
	dlg.ui.labelHeadline->setFont(SCScheme.fonts.highlight);
	dlg.ui.labelAuthor->setText("-");
	dlg.ui.labelDate->setText("-");

	setItalic(dlg.ui.labelAuthor);
	setItalic(dlg.ui.labelDate);

	QString oldComment;

	for ( size_t i = 0; i < SC_D.baseEvent->commentCount(); ++i ) {
		if ( SC_D.baseEvent->comment(i)->id() == "Operator" ) {
			try {
				dlg.ui.labelAuthor->setText(SC_D.baseEvent->comment(i)->creationInfo().author().c_str());
			}
			catch ( ... ) {}

			try {
				timeToLabel(dlg.ui.labelDate, SC_D.baseEvent->comment(i)->creationInfo().modificationTime(), "%F %T");
			}
			catch ( ... ) {
				try {
					timeToLabel(dlg.ui.labelDate, SC_D.baseEvent->comment(i)->creationInfo().creationTime(), "%F %T");
				}
				catch ( ... ) {}
			}

			oldComment = SC_D.baseEvent->comment(i)->text().c_str();
			dlg.ui.editComment->setPlainText(oldComment);

			break;
		}
	}

	if ( dlg.exec() != QDialog::Accepted ) return;

	if ( oldComment != dlg.ui.editComment->toPlainText() ) {
		sendJournal(SC_D.baseEvent->publicID(), "EvOpComment",
		            dlg.ui.editComment->toPlainText().toStdString());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commit(bool associate, bool ignoreDefaultEventType) {
	if ( SC_D.newOriginStatus != EvaluationStatus::Quantity ) {
		// In this case the commit has been triggered by the corresponding
		// button and not by "With additional options ..."
		OPT(EvaluationStatus) newStatus;

		try {
			if ( SC_D.currentOrigin->evaluationMode() == AUTOMATIC )
				newStatus = SC_D.newOriginStatus;
		}
		catch ( ... ) {
			// if evaluationMode isn't set yet we asume AUTOMATIC
			newStatus = SC_D.newOriginStatus;
		}

		if ( !SC_D.localOrigin ) {
			bool needConfirmation = false;

			if ( newStatus ) {
				try {
					if ( SC_D.currentOrigin->evaluationStatus() == *newStatus )
						needConfirmation = true;
				}
				catch ( ... ) {}
			}
			else
				needConfirmation = true;

			if ( needConfirmation ) {
				// This origin has not been changed
				int result = QMessageBox::question(
					this,
					tr("Confirm origin"),
					tr("Confirming an origin without changing its status will cause its author to be changed.\nDo you want to continue?"),
					QMessageBox::Yes, QMessageBox::No
				);

				if ( result != QMessageBox::Yes )
					return;
			}
		}

		if ( newStatus )
			SC_D.currentOrigin->setEvaluationStatus(*newStatus);
	}

	try {
		SC_D.currentOrigin->creationInfo();
	}
	catch( ... ) {
		SC_D.currentOrigin->setCreationInfo(CreationInfo());
	}

	CreationInfo &ci = SC_D.currentOrigin->creationInfo();
	ci.setAuthor( SCApp->author() );
	ci.setModificationTime(Core::Time::UTC());
	SC_D.ui.labelUser->setText(ci.author().c_str());
	SC_D.ui.labelUser->setToolTip(ci.author().c_str());

	// Update evaluation line
	QPalette pal = SC_D.ui.labelEvaluation->palette();
	pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
	SC_D.ui.labelEvaluation->setPalette(pal);

	QString evalMode;
	try {
		evalMode = SC_D.currentOrigin->evaluationStatus().toString();
		if ( SC_D.currentOrigin->evaluationStatus() == REJECTED ) {
			QPalette pal = SC_D.ui.labelEvaluation->palette();
			pal.setColor(QPalette::WindowText, Qt::red);
			SC_D.ui.labelEvaluation->setPalette(pal);
		}
	}
	catch ( ... ) {
		evalMode = "-";
	}

	try {
		if ( SC_D.currentOrigin->evaluationMode() == AUTOMATIC )
			evalMode += " (A)";
		else if ( SC_D.currentOrigin->evaluationMode() == MANUAL )
			evalMode += " (M)";
		else
			evalMode += " (-)";
	}
	catch ( ... ) {
		evalMode += " (-)";
	}

	SC_D.ui.labelEvaluation->setText(evalMode);

	if ( SC_D.recordView )
		SC_D.recordView->applyPicks();

	ObjectChangeList<DataModel::Pick> pickCommitList;
	std::vector<AmplitudePtr> amplitudeCommitList;

	// collect all picks belonging to the origin
	std::set<PickPtr> originPicks;
	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		PickPtr p = Pick::Find(SC_D.currentOrigin->arrival(i)->pickID());
		if ( p ) originPicks.insert(p);
	}

	// intersect the picks in the origin with the already tracked
	// manual created picks
	for ( PickSet::iterator it = SC_D.changedPicks.begin();
	      it != SC_D.changedPicks.end(); ++it ) {
		if ( originPicks.find(it->first) == originPicks.end() ) continue;
		pickCommitList.push_back(*it);
	}

	for ( AmplitudeSet::iterator it = SC_D.changedAmplitudes.begin();
	      it != SC_D.changedAmplitudes.end(); ++it ) {
		amplitudeCommitList.push_back(it->first);
	}

#if 0
	std::cerr << "PickList for commit:" << std::endl;
	for ( ObjectChangeList<DataModel::Pick>::iterator it = pickCommitList.begin();
		it != pickCommitList.end(); ++it ) {
		if ( it->second )
			std::cerr << "New:";
		else
			std::cerr << "Update:";
		std::cerr << " " << it->first->publicID() << std::endl;
	}
	std::cerr << "--------------------" << std::endl;

	std::cerr << "AmplitudeList for commit:" << std::endl;
	for ( std::vector<AmplitudePtr>::iterator it = amplitudeCommitList.begin();
		it != amplitudeCommitList.end(); ++it ) {
		std::cerr << "New: " << (*it)->publicID() << std::endl;
	}
	std::cerr << "--------------------" << std::endl;

#endif
	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		std::string pickID = SC_D.currentOrigin->arrival(i)->pickID();

		// try to find the pick somewhere in the client memory
		PickPtr pick = Pick::Find(pickID);
		if ( pick ) SC_D.associatedPicks[pickID] = pick;
	}

	if ( SC_D.localOrigin ) {
		// Strip invalid magnitudes
		size_t i = 0;
		while ( i < SC_D.currentOrigin->magnitudeCount() ) {
			Magnitude *mag = SC_D.currentOrigin->magnitude(i);
			try {
				// Only remove rejected magnitudes with no magnitude contributions.
				// There are cases when network magnitudes are invalid but
				// contain station magnitudes which did not pass QC checks.
				if ( mag->evaluationStatus() == REJECTED
				  && mag->stationMagnitudeContributionCount() == 0 ) {
					SC_D.currentOrigin->removeMagnitude(i);
					continue;
				}
			}
			catch ( ... ) {}
			++i;
		}
	}

	if ( !SC_D.localOrigin )
		emit updatedOrigin(SC_D.currentOrigin.get());
	else
		emit committedOrigin(SC_D.currentOrigin.get(),
		                     associate?SC_D.baseEvent.get():nullptr,
		                     pickCommitList, amplitudeCommitList);

	if ( !ignoreDefaultEventType && SC_D.baseEvent && SC_D.defaultEventType ) {
		// Check if event type changed
		bool typeSet;
		try { SC_D.baseEvent->type(); typeSet = true; }
		catch ( ... ) { typeSet = false; }

		if ( !typeSet )
			sendJournal(SC_D.baseEvent->publicID(), "EvType", SC_D.defaultEventType->toString());
	}

	SC_D.changedPicks.clear();
	SC_D.changedAmplitudes.clear();
	SC_D.localOrigin = false;

	//SC_D.ui.btnCommit->setEnabled(false);
	//SC_D.ui.btnCommit->setMenu(nullptr);
	SC_D.ui.btnMagnitudes->setEnabled(false);

	SC_D.undoList.clear();
	SC_D.redoList.clear();

	emit undoStateChanged(!SC_D.undoList.isEmpty());
	emit redoStateChanged(!SC_D.redoList.isEmpty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::customCommit() {
	CommitOptions customOptions = sender()->property("customCommit").value<CommitOptions>();
	if ( QApplication::keyboardModifiers() == Qt::ShiftModifier ) {
		customOptions.askForConfirmation = true;
	}

	customOptions.setup(SC_D.currentOrigin.get());

	QString fixedMagnitudeType = SC_D.actionCommitOptions->property("EvPrefMagType").toString();
	if ( !fixedMagnitudeType.isEmpty() ) {
		QMessageBox::StandardButton res = QMessageBox::NoButton;

		// Is there a magnitude type override
		if ( customOptions.magnitudeType ) {
			if ( *customOptions.magnitudeType != fixedMagnitudeType.toStdString() ) {
				res = QMessageBox::question(this,
					tr("Magnitude type"),
					tr("The commit requests to set the preferred magnitude type to '%1' "
					   "whereas the new preferred magnitude '%2' is currently selected.\n"
					   "Would you like to proceed with your selection?")
					.arg(customOptions.magnitudeType->c_str())
					.arg(fixedMagnitudeType),
					QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
				);
			}
		}
		else {
			res = QMessageBox::question(this,
				tr("Magnitude type"),
				tr("The new preferred magnitude '%1' is currently selected.\n"
				   "Would you like to proceed with your selection?")
				.arg(fixedMagnitudeType),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
			);
		}

		if ( res == QMessageBox::Cancel )
			return;

		if ( res == QMessageBox::Yes )
			customOptions.magnitudeType = fixedMagnitudeType.toStdString();
	}

	if ( customOptions.valid ) {
		commitWithOptions(&customOptions);
	}
	else {
		QMessageBox::critical(this, "Internal Error",
		                      tr("No options connected with commit button"));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commitFocalMechanism(bool withMT, QPoint pos) {
	if ( SC_D.localOrigin ) {
		QMessageBox::critical(this, "Commit",
		                      "The origin this focal mechanism uses as "
		                      "trigger is not yet committed.\n"
		                      "Commit the origin before committing the "
		                      "focal mechanism.");
		return;
	}

	MomentTensorPtr mt;
	OriginPtr derived;
	if ( withMT && SC_D.currentOrigin ) {
		OriginDialog dialog(SC_D.currentOrigin->longitude().value(),
		                    SC_D.currentOrigin->latitude().value(), this);
		try { dialog.setDepth(SC_D.currentOrigin->depth().value()); }
		catch ( ValueException &e ) {}
		dialog.setTime(SC_D.currentOrigin->time().value());

		dialog.enableAdvancedOptions(true, false);
		dialog.setPhaseCount(SC_D.residuals->count());
		// search for preferred magnitude value
		if ( SC_D.baseEvent ) {
			Magnitude *m = Magnitude::Find(SC_D.baseEvent->preferredMagnitudeID());
			if ( m )
				dialog.setMagValue(m->magnitude().value());
		}
		dialog.setMagType("Mw");

		if ( ! pos.isNull() )
			dialog.move(pos.x(), pos.y());

		if ( dialog.exec() != QDialog::Accepted ) return; // commit aborted

		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::UTC());

		// derive origin
		derived = Origin::Create();
		derived->setType(OriginType(HYPOCENTER));
		derived->setMethodID("FocalMechanism");
		derived->setCreationInfo(ci);
		derived->setEvaluationMode(EvaluationMode(MANUAL));

		try { derived->quality(); } // ensure existing quality object
		catch ( ValueException &e ) { derived->setQuality(OriginQuality()); }

		derived->setTime(Time(dialog.getTime_t(), 0));
		derived->setLatitude(dialog.latitude());
		derived->setLongitude(dialog.longitude());
		derived->setDepth(RealQuantity(dialog.depth()));
		derived->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
		derived->quality().setUsedPhaseCount(dialog.phaseCount());

		// moment magnitude
		MagnitudePtr mag = Magnitude::Create();
		mag->setCreationInfo(ci);
		mag->setMagnitude(dialog.magValue());
		mag->setType(dialog.magType().toStdString());
		mag->setOriginID(derived->publicID());
		mag->setStationCount(dialog.phaseCount());
		derived->add(mag.get());

		// moment tensor
		mt = MomentTensor::Create();
		mt->setDerivedOriginID(derived->publicID());
		mt->setMomentMagnitudeID(mag->publicID());
		mt->setCreationInfo(ci);
	}

	// Create FocalMechanism of both nodal planes
	NODAL_PLANE np1 = static_cast<PlotWidget*>(SC_D.residuals)->np1();
	NODAL_PLANE np2 = static_cast<PlotWidget*>(SC_D.residuals)->np2();

	FocalMechanismPtr fm = FocalMechanism::Create();
	fm->setTriggeringOriginID(SC_D.currentOrigin->publicID());
	if ( mt )
		fm->add(mt.get());
	NodalPlanes nps;
	NodalPlane np;
	np.setStrike(np1.str);
	np.setDip(np1.dip);
	np.setRake(np1.rake);
	nps.setNodalPlane1(np);
	np.setStrike(np2.str);
	np.setDip(np2.dip);
	np.setRake(np2.rake);
	nps.setNodalPlane2(np);
	fm->setNodalPlanes(nps);
	fm->setMethodID("first motion");
	fm->setEvaluationMode(EvaluationMode(MANUAL));
	fm->setEvaluationStatus(EvaluationStatus(CONFIRMED));

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::UTC());

	fm->setCreationInfo(ci);

	if ( fm )
		emit committedFocalMechanism(fm.get(), SC_D.baseEvent.get(),
		                             derived?derived.get():nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commitWithOptions() {
	CommitOptions options;

	// Setup options
	options.init("olv.commit.", SC_D.currentOrigin.get());
	options.askForConfirmation = true;

	options.magnitudeType = SC_D.actionCommitOptions->property("EvPrefMagType").toString().toStdString();
	options.eventType = SC_D.defaultEventType;

	if ( SC_D.baseEvent ) {
		try { options.eventType = SC_D.baseEvent->type(); }
		catch ( ... ) {}
		try { options.eventTypeCertainty = SC_D.baseEvent->typeCertainty(); }
		catch ( ... ) {}

		if ( SC_D.reader && SC_D.baseEvent->eventDescriptionCount() == 0 )
			SC_D.reader->loadEventDescriptions(SC_D.baseEvent.get());

		if ( SC_D.reader && SC_D.baseEvent->commentCount() == 0 )
			SC_D.reader->loadComments(SC_D.baseEvent.get());

		// Fill earthquake name
		for ( size_t i = 0; i < SC_D.baseEvent->eventDescriptionCount(); ++i ) {
			if ( SC_D.baseEvent->eventDescription(i)->type() == EARTHQUAKE_NAME ) {
				options.eventName = SC_D.baseEvent->eventDescription(i)->text().c_str();
				break;
			}
		}

		// Fill operator's comment
		for ( size_t i = 0; i < SC_D.baseEvent->commentCount(); ++i ) {
			if ( SC_D.baseEvent->comment(i)->id() == "Operator" ) {
				options.eventComment = SC_D.baseEvent->comment(i)->text();
				break;
			}
		}
	}

	try {
		options.originStatus = SC_D.currentOrigin->evaluationStatus();
	}
	catch ( ... ) {
		options.originStatus = OPT(EvaluationStatus)(CONFIRMED);
	}

	commitWithOptions(&options);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commitWithOptions(const void *data_ptr) {
	const CommitOptions *options_ptr = reinterpret_cast<const CommitOptions*>(data_ptr);
	CommitOptions tmp;
	bool dialogConfirmed = false;

	if ( options_ptr->askForConfirmation ) {
		OriginCommitOptions dlg;
		tmp = *options_ptr;
		dlg.setOptions(tmp, SC_D.baseEvent.get(), SC_D.localOrigin);
		if ( dlg.exec() != QDialog::Accepted ) {
			return;
		}
		if ( !dlg.getOptions(tmp) ) {
			return;
		}

		options_ptr = &tmp;

		if ( dlg.ui.comboPreferredMagnitude->currentIndex() > 1 ) {
			tmp.magnitudeType = dlg.ui.comboPreferredMagnitude->currentData().toString().toStdString();
		}
		else if ( dlg.ui.comboPreferredMagnitude->currentIndex() == 1 ) {
			// Force unsetting the preferred magnitude
			tmp.magnitudeType = string();
		}

		dialogConfirmed = true;
	}

	const CommitOptions &options = *options_ptr;
	bool isLocalOrigin = SC_D.localOrigin;

	if ( options.originStatus ) {
		if ( SC_D.localOrigin ) {
			SC_D.currentOrigin->setEvaluationStatus(*options.originStatus);

			// Do not override the status in commit
			SC_D.newOriginStatus = EvaluationStatus::Quantity;
			commit(options.forceEventAssociation, true);
			SC_D.newOriginStatus = CONFIRMED;
		}
		else {
			OPT(EvaluationStatus) os;
			try { os = SC_D.currentOrigin->evaluationStatus(); } catch ( ... ) {}

			if ( os != *options.originStatus ) {
				SC_D.currentOrigin->setEvaluationStatus(*options.originStatus);

				SC_D.newOriginStatus = EvaluationStatus::Quantity;
				commit(true, true);
				SC_D.newOriginStatus = CONFIRMED;
			}
		}
	}

	// wait for event
	if ( !SC_D.baseEvent || (!options.forceEventAssociation && isLocalOrigin) ) {
		cerr << "Wait for association" << endl;
		QProgressDialog progress("Origin has not been associated with an event yet.\n"
		                         "Waiting for event association ...\n"
		                         "Hint: scevent should run",
		                         "Cancel", 0, 0);
		progress.setAutoClose(true);
		progress.setWindowModality(Qt::ApplicationModal);
		connect(this, SIGNAL(baseEventSet()), &progress, SLOT(accept()));
		connect(this, SIGNAL(baseEventRejected()), &progress, SLOT(reject()));
		if ( progress.exec() != QDialog::Accepted )
			return;
	}

	// Do event specific things
	if ( SC_D.baseEvent ) {
		string type, newType, typeCertainty, newTypeCertainty;
		string name, comment;

		if ( options.eventType ) {
			newType = options.eventType->toString();
		}

		try { type = SC_D.baseEvent->type().toString(); }
		catch ( ... ) {}

		if ( options.eventTypeCertainty ) {
			newTypeCertainty = options.eventTypeCertainty->toString();
		}

		try { typeCertainty = SC_D.baseEvent->typeCertainty().toString(); }
		catch ( ... ) {}

		EventDescription *desc = SC_D.baseEvent->eventDescription(EventDescriptionIndex(EARTHQUAKE_NAME));
		if ( desc ) {
			name = desc->text();
		}

		Comment *cmt = SC_D.baseEvent->comment(CommentIndex("Operator"));
		if ( cmt ) {
			comment = cmt->text();
		}

		NotifierMessagePtr nm = new NotifierMessage;

		if ( comment != options.eventComment ) {
			nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvOpComment", options.eventComment));
		}

		if ( name != options.eventName ) {
			nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvName", options.eventName));
		}

		if ( type != newType ) {
			nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvType", newType));
		}

		if ( typeCertainty != newTypeCertainty ) {
			nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvTypeCertainty", newTypeCertainty));
		}

		if ( options.fixOrigin ) {
			nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvPrefOrgID", SC_D.currentOrigin->publicID()));
		}

		if ( options.magnitudeType ) {
			if ( !options.magnitudeType->empty() ) {
				// Only attach the fix command if the requested type is not fixed already
				if ( SC_D.reader ) {
					DatabaseIterator it;
					string lastFix;
					Time lastFixCreated;
					it = SC_D.reader->getJournalAction(SC_D.baseEvent->publicID(), "EvPrefMagType");
					while ( *it ) {
						auto entry = static_cast<JournalEntry*>(*it);
						if ( !lastFixCreated.valid() || lastFixCreated < entry->created() ) {
							lastFix = entry->parameters();
							lastFixCreated = entry->created();
						}
						++it;
					}
					it.close();

					if ( lastFix != *options.magnitudeType ) {
						nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvPrefMagType", *options.magnitudeType));
					}
				}
				else {
					nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvPrefMagType", *options.magnitudeType));
				}
			}
			else {
				// Only attach the unfix command if currently a type is fixed
				if ( SC_D.reader ) {
					DatabaseIterator it;
					string lastFix;
					Time lastFixCreated;
					it = SC_D.reader->getJournalAction(SC_D.baseEvent->publicID(), "EvPrefMagType");
					while ( *it ) {
						auto entry = static_cast<JournalEntry*>(*it);
						if ( !lastFixCreated.valid() || lastFixCreated < entry->created() ) {
							lastFix = entry->parameters();
							lastFixCreated = entry->created();
						}
						++it;
					}
					it.close();

					if ( !lastFix.empty() ) {
						nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvPrefMagType", ""));
					}
				}
				else {
					nm->attach(createJournal(SC_D.baseEvent->publicID(), "EvPrefMagType", ""));
				}
			}
		}

		if ( dialogConfirmed ) {
			// Only if the dialog was opened the synchronization of origin
			// comments is allowed.

			for ( const auto &profile : options.originCommentProfiles ) {
				CommentPtr comment = SC_D.currentOrigin->comment(profile.id);

				if ( profile.value.empty() ) {
					if ( comment ) {
						SC_D.currentOrigin->remove(comment.get());
						if ( !Notifier::IsEnabled()) {
							nm->attach(
								new Notifier(
									SC_D.currentOrigin->publicID(),
									OP_REMOVE, comment.get()
								)
							);
						}
					}
				}
				else {
					if ( comment ) {
						if ( profile.value != comment->text() ) {
							comment->setText(profile.value);
							comment->update();
							if ( !Notifier::IsEnabled()) {
								nm->attach(
									new Notifier(
										SC_D.currentOrigin->publicID(),
										OP_UPDATE, comment.get()
									)
								);
							}
						}
					}
					else {
						comment = new Comment;
						comment->setId(profile.id);
						comment->setText(profile.value);
						SC_D.currentOrigin->add(comment.get());
						if ( !Notifier::IsEnabled()) {
							nm->attach(
								new Notifier(
									SC_D.currentOrigin->publicID(),
									OP_ADD, comment.get()
								)
							);
						}
					}
				}
			}
		}

		if ( !nm->empty() ) {
			if ( SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get()) ) {
				NotifierMessage::iterator it;
				for ( it = nm->begin(); it != nm->end(); ++ it )
					SCApp->emitNotifier(it->get());
			}
		}
	}

	if ( options.returnToEventList ) {
		emit eventListRequested();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Notifier *
OriginLocatorView::createJournal(const std::string &objectID,
                                 const std::string &action,
                                 const std::string &params) {
	/*
	if ( _updateLocalEPInstance ) {
		NotifierPtr notifier;

		if ( action == "EvType" ) {
			EventType et;
			if ( et.fromString(params) )
				_currentEvent->setType(et);
			else
				_currentEvent->setType(Core::None);

			notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
		}
		else if ( action == "EvPrefOrgID" ) {
			if ( params.empty() )
				QMessageBox::critical(this, "Error", "Releasing the preferred origin in offline mode is not supported.");
			else if ( _currentEvent->preferredOriginID() != params ) {
				_currentEvent->setPreferredOriginID(params);
				_currentEvent->setPreferredMagnitudeID("");
				notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
			}
		}

		if ( notifier ) SCApp->emitNotifier(notifier.get());
	}
	else */ {
		JournalEntryPtr entry = new JournalEntry;
		entry->setObjectID(objectID);
		entry->setAction(action);
		entry->setParameters(params);
		entry->setSender(SCApp->author());
		entry->setCreated(Core::Time::UTC());
		return new Notifier("Journaling", OP_ADD, entry.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::sendJournal(const std::string &objectID,
                                    const std::string &action,
                                    const std::string &params) {
	NotifierPtr n = createJournal(objectID, action, params);
	NotifierMessagePtr nm = new NotifierMessage;
	nm->attach(n.get());
	if ( SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get()) ) {
		SCApp->emitNotifier(n.get());
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct InvertFilter : ArrivalModel::Filter {
	InvertFilter(QItemSelectionModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *arr) const {
		return !_model->isRowSelected(row, QModelIndex());
	}

	QItemSelectionModel *_model;
};


struct AutomaticPickFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		Pick *p = Pick::Find(arr->pickID());
		if ( p == nullptr ) return false;

		try {
			return p->evaluationMode() == AUTOMATIC;
		}
		catch ( ... ) {
			return true;
		}
	}
};


struct ManualPickFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		Pick *p = Pick::Find(arr->pickID());
		if ( p == nullptr ) return false;

		try {
			return p->evaluationMode() == MANUAL;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct ZeroWeightFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		try {
			return arr->weight() == 0.0;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct NonZeroWeightFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		try {
			return arr->weight() != 0.0;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct ActivatedArrivalFilter : ArrivalModel::Filter {
	ActivatedArrivalFilter(QAbstractItemModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *) const {
		return _model->data(_model->index(row, 0), UsedRole).toInt() != 0;
	}

	QAbstractItemModel *_model;
};


struct DeactivatedArrivalFilter : ArrivalModel::Filter {
	DeactivatedArrivalFilter(QAbstractItemModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *) const {
		return _model->data(_model->index(row, 0), UsedRole).toInt() == 0;
	}

	QAbstractItemModel *_model;
};


}

void OriginLocatorView::tableArrivalsContextMenuRequested(const QPoint &pos) {
	QMenu menu;

	if ( !SC_D.ui.tableArrivals->selectionModel() ) return;
	bool hasSelection = SC_D.ui.tableArrivals->selectionModel()->hasSelection();

	QAction *actionInvertSelection = menu.addAction("Invert selection");
	QMenu *subSelection = menu.addMenu("Select");
	QAction *actionSelectAutomatic = subSelection->addAction("Automatic picks");
	QAction *actionSelectManual = subSelection->addAction("Manual picks");
	subSelection->addSeparator();
	QAction *actionSelectWeight0 = subSelection->addAction("Zero weight");
	QAction *actionSelectWeightNon0 = subSelection->addAction("Non-zero weight");
	subSelection->addSeparator();
	QAction *actionSelectActivated = subSelection->addAction("Activated");
	QAction *actionSelectDeactivated = subSelection->addAction("Deactivated");

	menu.addSeparator();

	QMenu *subActivate = menu.addMenu("Activate");
	QMenu *subDeactivate = menu.addMenu("Deactivate");

	QAction *actionActivate = subActivate->addAction("All");
	QAction *actionActivateTime = subActivate->addAction("Time");
	QAction *actionActivateBaz = subActivate->addAction("Backazimuth");
	QAction *actionActivateSlow = subActivate->addAction("Slowness");

	QAction *actionDeactivate = subDeactivate->addAction("All");
	QAction *actionDeactivateTime = subDeactivate->addAction("Time");
	QAction *actionDeactivateBaz = subDeactivate->addAction("Backazimuth");
	QAction *actionDeactivateSlow = subDeactivate->addAction("Slowness");

	if ( !hasSelection ) {
		actionActivate->setEnabled(false);
		actionDeactivate->setEnabled(false);
	}

	menu.addSeparator();

	QAction *actionRename = menu.addAction("Rename selected arrivals");

	if ( !hasSelection )
		actionRename->setEnabled(false);

	menu.addSeparator();

	QAction *actionDeleteSelectedArrivals = menu.addAction("Delete selected arrivals");
	if ( !hasSelection )
		actionDeleteSelectedArrivals->setEnabled(false);

	menu.addSeparator();
	QAction *actionCopyCellClipboard = menu.addAction("Copy cell to clipboard");
	QAction *actionCopyToClipboard = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(SC_D.ui.tableArrivals->mapToGlobal(pos));

	if ( result == actionDeleteSelectedArrivals )
		deleteSelectedArrivals();
	else if ( result == actionActivate )
		activateSelectedArrivals(Seismology::LocatorInterface::F_ALL, true);
	else if ( result == actionActivateTime )
		activateSelectedArrivals(Seismology::LocatorInterface::F_TIME, true);
	else if ( result == actionActivateBaz )
		activateSelectedArrivals(Seismology::LocatorInterface::F_BACKAZIMUTH, true);
	else if ( result == actionActivateSlow )
		activateSelectedArrivals(Seismology::LocatorInterface::F_SLOWNESS, true);
	else if ( result == actionDeactivate )
		activateSelectedArrivals(Seismology::LocatorInterface::F_ALL, false);
	else if ( result == actionDeactivateTime )
		activateSelectedArrivals(Seismology::LocatorInterface::F_TIME, false);
	else if ( result == actionDeactivateBaz )
		activateSelectedArrivals(Seismology::LocatorInterface::F_BACKAZIMUTH, false);
	else if ( result == actionDeactivateSlow )
		activateSelectedArrivals(Seismology::LocatorInterface::F_SLOWNESS, false);

	else if ( result == actionInvertSelection )
		selectArrivals(InvertFilter(SC_D.ui.tableArrivals->selectionModel()));
	else if ( result == actionSelectAutomatic )
		selectArrivals(AutomaticPickFilter());
	else if ( result == actionSelectManual )
		selectArrivals(ManualPickFilter());
	else if ( result == actionSelectWeight0 )
		selectArrivals(ZeroWeightFilter());
	else if ( result == actionSelectWeightNon0 )
		selectArrivals(NonZeroWeightFilter());
	else if ( result == actionSelectActivated )
		selectArrivals(ActivatedArrivalFilter(SC_D.modelArrivalsProxy));
	else if ( result == actionRename )
		renameArrivals();
	else if ( result == actionSelectDeactivated )
		selectArrivals(DeactivatedArrivalFilter(SC_D.modelArrivalsProxy));
	else if ( result == actionCopyCellClipboard ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb ) {
			int column = SC_D.ui.tableArrivals->columnAt(pos.x());
			int row = SC_D.ui.tableArrivals->rowAt(pos.y());
			cb->setText(
				SC_D.ui.tableArrivals->model()->data(
					SC_D.ui.tableArrivals->model()->index(row, column)
				).toString()
			);
		}
	}
	else if ( result == actionCopyToClipboard )
		SCApp->copyToClipboard(SC_D.ui.tableArrivals);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::tableArrivalsHeaderContextMenuRequested(const QPoint &pos) {
	int count = SC_D.ui.tableArrivals->horizontalHeader()->count();
	QAbstractItemModel *model = SC_D.ui.tableArrivals->horizontalHeader()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!SC_D.ui.tableArrivals->horizontalHeader()->isSectionHidden(i));
	}

	QAction *result = menu.exec(SC_D.ui.tableArrivals->horizontalHeader()->mapToGlobal(pos));
	if ( result == nullptr ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		colVisibility[i] = actions[i]->isChecked();

	SC_D.ui.tableArrivals->horizontalHeader()->setSectionHidden(section, !colVisibility[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrivals(const ArrivalModel::Filter *f) {
	if ( SC_D.ui.tableArrivals->selectionModel() == nullptr )
		return;

	QItemSelection selection;

	for ( int i = 0; i < SC_D.modelArrivalsProxy->rowCount(); ++i ) {
		Arrival *arr = SC_D.currentOrigin->arrival(i);
		QModelIndex idx = SC_D.modelArrivalsProxy->mapFromSource(SC_D.modelArrivals.index(i,0));

		if ( f != nullptr && !f->accepts(idx.row(), i, arr) ) continue;

		selection.append(QItemSelectionRange(idx));
	}

	//selection = SC_D.modelArrivalsProxy->mapSelectionFromSource(selection);
	SC_D.ui.tableArrivals->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrivals(const ArrivalModel::Filter &f) {
	selectArrivals(&f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::deleteSelectedArrivals() {
	if ( SC_D.ui.tableArrivals->selectionModel() == nullptr )
		return;

	QModelIndexList rows = SC_D.ui.tableArrivals->selectionModel()->selectedRows();
	if ( rows.empty() ) return;

	if ( SC_D.currentOrigin == nullptr ) return;

	OriginPtr origin = Origin::Create();
	*origin = *SC_D.currentOrigin;

	vector<bool> flags;
	flags.resize(SC_D.currentOrigin->arrivalCount(), false);

	foreach ( const QModelIndex &idx, rows ) {
		int row = SC_D.modelArrivalsProxy->mapToSource(idx).row();
		if ( row >= (int)SC_D.currentOrigin->arrivalCount() ) {
			cerr << "Delete arrivals: invalid idx " << row
			     << ": only " << SC_D.currentOrigin->arrivalCount() << " available"
			     << endl;
			continue;
		}

		flags[row] = true;
	}

	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		// Skip arrivals to be deleted
		if ( flags[i] ) continue;

		// Copy existing arrivals
		Arrival *arr = SC_D.currentOrigin->arrival(i);
		ArrivalPtr newArr = new Arrival(*arr);
		origin->add(newArr.get());
	}

	applyNewOrigin(origin.get(), false);
	startBlinking(QColor(255,128,0), SC_D.ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::activateSelectedArrivals(Seismology::LocatorInterface::Flags flags,
                                                 bool activate) {
	if ( SC_D.ui.tableArrivals->selectionModel() == nullptr )
		return;

	QModelIndexList rows = SC_D.ui.tableArrivals->selectionModel()->selectedRows();
	if ( rows.empty() ) return;

	bool changed = false;

	foreach ( const QModelIndex &idx, rows ) {
		int mask = getMask(idx);
		int oldFlags = idx.data(UsedRole).toInt();
		int newFlags = oldFlags;
		if ( activate )
			newFlags |= flags;
		else
			newFlags &= ~flags;
		newFlags &= mask;

		if ( oldFlags != newFlags ) {
			SC_D.modelArrivalsProxy->setData(idx, newFlags, UsedRole);
			changed = true;
		}
	}

	if ( changed )
		startBlinking(QColor(255,128,0), SC_D.ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::renameArrivals() {
	RenamePhases dlg(this);

	QSet<QString> phases;
	QSet<Arrival*> arrivals;

	for ( int i = 0; i < SC_D.modelArrivalsProxy->rowCount(); ++i ) {
		Arrival *arr = SC_D.currentOrigin->arrival(i);
		QModelIndex idx = SC_D.modelArrivalsProxy->mapFromSource(SC_D.modelArrivals.index(i,0));

		if ( SC_D.ui.tableArrivals->selectionModel()->isRowSelected(idx.row(), QModelIndex()) ) {
			phases.insert(arr->phase().code().c_str());
			arrivals.insert(arr);
		}
	}

	foreach (const QString &ph, phases)
		dlg.ui.listSourcePhases->addItem(ph);

	PickerView::Config::StringList pickPhases;
	SC_D.pickerConfig.getPickPhases(pickPhases);
	foreach (const QString &ph, pickPhases)
		dlg.ui.listTargetPhase->addItem(ph);

	if ( dlg.exec() != QDialog::Accepted )
		return;

	QList<QListWidgetItem*> sourceItems = dlg.ui.listSourcePhases->selectedItems();
	if ( sourceItems.empty() ) {
		QMessageBox::information(this, "Rename arrivals",
		                         "No source phases selected: nothing to do.");
		return;
	}

	QList<QListWidgetItem*> targetItems = dlg.ui.listTargetPhase->selectedItems();
	if ( targetItems.empty() ) {
		QMessageBox::information(this, "Rename arrivals",
		                         "No target phase selected: nothing to do.");
		return;
	}

	if ( targetItems.count() > 1 ) {
		QMessageBox::critical(this, "Rename arrivals",
		                      "Internal error: More than one target phase selected.");
		return;
	}

	if ( SC_D.currentOrigin == nullptr ) return;

	OriginPtr origin = Origin::Create();
	*origin = *SC_D.currentOrigin;

	phases.clear();
	foreach ( QListWidgetItem *item, sourceItems )
		phases.insert(item->text());

	for ( size_t i = 0; i < SC_D.currentOrigin->arrivalCount(); ++i ) {
		Arrival *arr = SC_D.currentOrigin->arrival(i);

		// Copy existing arrivals
		ArrivalPtr newArr = new Arrival(*arr);

		if ( arrivals.contains(arr) && phases.contains(arr->phase().code().c_str()) )
			newArr->setPhase(Phase(targetItems[0]->text().toStdString()));

		origin->add(newArr.get());
	}

	applyNewOrigin(origin.get(), false);
	startBlinking(QColor(255,128,0), SC_D.ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::dataChanged(const QModelIndex& topLeft, const QModelIndex&) {
	if ( topLeft.column() != USED ) return;

	int flags = SC_D.modelArrivals.data(topLeft, UsedRole).toInt();
	bool used = flags != 0;
	SC_D.residuals->setValueSelected(topLeft.row(), used);
	SC_D.map->setArrivalState(topLeft.row(), used);
	if ( SC_D.toolMap )
		SC_D.toolMap->setArrivalState(topLeft.row(), used);
	if ( SC_D.recordView )
		SC_D.recordView->setArrivalState(topLeft.row(), used);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evalResultAvailable(const QString &oid,
                                            const QString &className,
                                            const QString &script,
                                            const QString &result) {
	if ( !SC_D.currentOrigin || SC_D.currentOrigin->publicID() != oid.toStdString() ) {
		return;
	}

	auto it = SC_D.scriptLabelMap.find(script);
	if ( it == SC_D.scriptLabelMap.end() ) {
		return;
	}

	it.value().first->setEnabled(true);
	it.value().second->setText(result);

	it.value().second->setPalette(SC_D.ui.labelEventID->palette());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evalResultError(const QString &oid,
                                        const QString &className,
                                        const QString &script,
                                        int error) {
	if ( !SC_D.currentOrigin || SC_D.currentOrigin->publicID() != oid.toStdString() ) {
		return;
	}

	auto it = SC_D.scriptLabelMap.find(script);
	if ( it == SC_D.scriptLabelMap.end() ) {
		return;
	}

	it.value().first->setEnabled(true);
	it.value().second->setText("ERROR");
	QPalette p = it.value().second->palette();
	p.setColor(QPalette::WindowText, Qt::darkRed);
	it.value().second->setPalette(p);
	it.value().second->setToolTip(PublicObjectEvaluator::Instance().errorMsg(error));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evaluateOrigin(Seiscomp::DataModel::Origin *org,
                                       Seiscomp::DataModel::Event *,
                                       bool localOrigin, bool) {
	QStringList scripts;
	for ( auto it = SC_D.scriptLabelMap.begin(); it != SC_D.scriptLabelMap.end(); ++it )
		scripts << it.key();

	// Local origins need special handling
	if ( localOrigin ) {
		PublicObjectEvaluator::Instance().eval(org, scripts);
	}
	else {
		PublicObjectEvaluator::Instance().prepend(this, org->publicID().c_str(),
		                                          org->typeInfo(), scripts);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}


Q_DECLARE_METATYPE(CommitOptions)
