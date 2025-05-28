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


#include <seiscomp/gui/core/xmlview.h>
#include <QXmlDefaultHandler>
#include <QtXml>


namespace Seiscomp {
namespace Gui {


class XmlHandler : public QXmlDefaultHandler {
	public:
		XmlHandler(QTreeWidget* w)
		 : QXmlDefaultHandler(), _tree(w), _parent(nullptr), _currentItem(nullptr) {}

		bool startElement(const QString& /*namespaceURI*/,
		                  const QString& localName,
		                  const QString& /*qName*/,
		                  const QXmlAttributes& atts) {

			_currentItem = new QTreeWidgetItem();
			_currentItem->setText(0, localName);
			if ( _parent == nullptr )
				_tree->addTopLevelItem(_currentItem);
			else
				_parent->addChild(_currentItem);

			QFont font(_currentItem->font(0));
			font.setBold(true);
			_currentItem->setFont(0, font);

			_parent = _currentItem;

			for ( int i = 0; i < atts.count(); ++i ) {
				QTreeWidgetItem* item = new QTreeWidgetItem(_parent);
				item->setText(0, atts.localName(i));
				item->setText(1, atts.value(i));
				QFont font(item->font(0));
				//font.setBold(true);
				font.setItalic(true);
				item->setFont(0, font);
			}

			_tree->expandItem(_parent);

			return true;
		}

		bool characters(const QString & ch) {
			if ( _currentItem ) {
				_currentItem->setText(1, ch);
			}

			return true;
		}

		bool endElement(const QString& /*namespaceURI*/,
		                const QString& /*localName*/,
		                const QString& /*qName*/) {
			if ( _parent != nullptr )
				_parent = _parent->parent();

			_currentItem = nullptr;

			return true;
		}

		bool endDocument() {
			//_tree->adjustSize();
			_tree->resizeColumnToContents(0);
			_tree->header()->resizeSection(0, _tree->header()->sectionSize(0) + 10);
			return true;
		}

	private:
		QTreeWidget* _tree;
		QTreeWidgetItem* _parent;
		QTreeWidgetItem* _currentItem;
};


XMLView::XMLView(QWidget * parent, Qt::WindowFlags f, bool deleteOnClose)
: QWidget(parent, f) {
	if ( deleteOnClose )
		setAttribute(Qt::WA_DeleteOnClose);

	_ui.setupUi(this);
	_ui.treeWidget->setColumnCount(2);
	QTreeWidgetItem* header = new QTreeWidgetItem();
	header->setText(0, tr("Name"));
	header->setText(1, tr("Value"));
	_ui.treeWidget->setHeaderItem(header);
}

XMLView::~XMLView() {
}

void XMLView::setContent(const QString& content) {
	_ui.treeWidget->clear();
	//_ui.treeWidget->adjustSize();

	QXmlInputSource source;
	source.setData(content);

	QXmlSimpleReader xmlReader;
	XmlHandler *handler = new XmlHandler(_ui.treeWidget);

	xmlReader.setContentHandler(handler);
	xmlReader.setErrorHandler(handler);
	xmlReader.parse(&source);

	delete handler;
}


}
}
