#include <QApplication>
#include <QScreen>
#include <QGraphicsScene>
#include <QXmlStreamWriter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QPainter>

#include "TextItem.h"


class InternalTextItem : public QGraphicsTextItem {
public:
    InternalTextItem(QGraphicsItem * parent) : QGraphicsTextItem(parent) {
        // calculate the scale for the text item - this is the same
        // calculation as in GraphicsSheet::GraphicsSheet, so this should
        // be located in some common convenience method
        QScreen *srn = QApplication::screens().at(0);
        qreal scale = 25.4 / srn->logicalDotsPerInchY();
        setScale(scale);
    }


    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0 ){
        QStyleOptionGraphicsItem option2 = *option;
        option2.state = 0;
        QGraphicsTextItem::paint(painter, &option2, widget);
    }


    // @Override
    virtual void focusOutEvent ( QFocusEvent * event ) {
    	// clear the current selection
    	// See also http://qt-project.org/forums/viewthread/7322
    	QTextCursor t = textCursor();
    	t.clearSelection();
    	setTextCursor(t);

    	QGraphicsTextItem::focusOutEvent(event);
    }


    int type () const {
        return InternalTextItemType;
    }
};


TextItem::TextItem(const QPoint& pos, QGraphicsItem * parent) :
        RectItem(QRectF(pos.x(), pos.y(), 50, 30), parent), alignment(Qt::AlignHCenter) {

    textItem = new InternalTextItem (this);
    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);

    // This item is focusable and passes the focus on to the child item
    setFlag (QGraphicsItem::ItemIsFocusable);
    setFocusProxy(textItem);

    setFiltersChildEvents(true);
    setInternalAlignment(Qt::AlignHCenter);

    centerTextItem();
}

#if 0
void EditableTextItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {

	// paint border if not in preview mode - selection border is drawn by the interactor decorations
    KollageGraphicsScene* theScene = dynamic_cast<KollageGraphicsScene*>(scene());
	if (!isSelected() && theScene->getEditMode() != KollageGraphicsScene::MODE_PREVIEW) {
		painter->setPen(QPen(Qt::gray, 0, Qt::DashLine));
		painter->drawRect(rect());
	}

    // overlay edit markers
    EditableItem::paint(painter, option, widget);
}


void EditableTextItem::accept(const ItemVisitor& visitor) {
    visitor.visit(this);
}


int EditableTextItem::type () const {
    return EditableTextItemType;
}


QFont EditableTextItem::font() const {
    return textItem->font();
}

// TODO: Maybe it is better to create the SetFontCommand outside of this class -
// same for the text color command ...
void EditableTextItem::setFont(const QFont& font) {
    KollageGraphicsScene* theScene = dynamic_cast<KollageGraphicsScene*>(scene());
    QUndoCommand* undo = new SetFontCommand(this, font);
    theScene->getUndoStack()->push(undo);
}

#endif

void TextItem::setInternalFont(const QFont& font) {
    textItem->setFont(font);
    centerTextItem();
}

void TextItem::setText(const QString& text) {
    textItem->setPlainText(text);
    centerTextItem();
    setInternalAlignment(alignment);
}


void TextItem::centerTextItem() {
    // set the text width to the whole EditableTextItem width
	// !!!! http://www.cesarbs.org/blog/2011/05/30/aligning-text-in-qgraphicstextitem
    textItem->setTextWidth(rect().width() / textItem->scale());

    // vertically center the text item
    QRectF textBound = textItem->boundingRect();
    qreal textRectHeight = textBound.height() * textItem->scale();
    QPointF centerPos(0, (rect().height() - textRectHeight) / 2);
    textItem->setPos(centerPos);
}


void TextItem::moveHandle(AbstractEditHandle editHandle, const QPointF& scenePos) {
    RectItem::moveHandle(editHandle, scenePos);
    centerTextItem();
}


void TextItem::setInternalAlignment(Qt::Alignment align) {
	// Store alignment
	alignment = align;

    // Set the alignment of all blocks to the given alignment
    QTextDocument* textDocument = textItem->document();
    for (QTextBlock it = textDocument->begin(); it != textDocument->end(); it = it.next()) {
		QTextCursor cur(it);
		QTextBlockFormat format = cur.blockFormat();
		format.setAlignment(align);
		cur.setBlockFormat(format);
    }
}

#if 0
void TextItem::setAlignment(Qt::Alignment align) {
    KollageGraphicsScene* theScene = dynamic_cast<KollageGraphicsScene*>(scene());
    QUndoCommand* undo = new SetAlignmentCommand(this, align);
    theScene->getUndoStack()->push(undo);
}
#endif

Qt::Alignment TextItem::getAlignment() {
    return alignment;
}

#if 0
void TextItem::setTextColor(const QColor& col) {
    KollageGraphicsScene* theScene = dynamic_cast<KollageGraphicsScene*>(scene());
    QUndoCommand* undo = new TextColorCommand(this, col);
    theScene->getUndoStack()->push(undo);
}
#endif

void TextItem::setInternalDefaultTextColor(const QColor& col) {
    textItem->setDefaultTextColor(col);
}


QColor TextItem::getTextColor() {
    return textItem->defaultTextColor();
}

#if 0
// TODO: The generic part (x/y/w/h/rot) should be done in the super class
EditableTextItem* EditableTextItem::readExternal(QXmlStreamReader& reader) {
    EditableTextItem* item = 0;

    QString xpos = reader.attributes().value("xpos").toString();
    QString ypos = reader.attributes().value("ypos").toString();
    QString width = reader.attributes().value("width").toString();
    QString height = reader.attributes().value("height").toString();
    QString rotationAttr = reader.attributes().value("rotation").toString();

    item = new EditableTextItem(QPoint(xpos.toInt(), ypos.toInt()));
    item->setRect(QRect(0, 0, width.toInt(), height.toInt()));

    QPointF center = QPointF(item->rect().width() / 2, item->rect().height() / 2);
    item->setTransformOriginPoint(center);
    item->setRotation(rotationAttr.toInt());

    reader.readNext();  // discard characters for textFrame

    reader.readNext();
    if (reader.name() == "text") {
    	// read font parameters and set the font
        QString fontDesc = reader.attributes().value("font").toString();
        QFont font;
        font.fromString(fontDesc);
        item->setInternalFont(font);

        // read alignment, and check valid values
        int alignDesc = reader.attributes().value("align").toString().toInt();
        switch(alignDesc) {
			case Qt::AlignLeft : break;
			case Qt::AlignHCenter : break;
			case Qt::AlignRight : break;
			default : alignDesc = Qt::AlignLeft; break;
        }

        // read text color and set it
        QString bgColor = reader.attributes().value("textColor").toString();
        item->setInternalDefaultTextColor(QColor(bgColor));

        // read text and set it
        reader.readNext();
        item->setText(reader.text().toString());

        // now set alignment - needs text to be effective!
        item->setInternalAlignment(static_cast<Qt::Alignment>(alignDesc));
    }

    return item;
}


void EditableTextItem::writeExternal(QXmlStreamWriter& writer) {
    writer.writeStartElement("textFrame");

    writer.writeAttribute("xpos", QString::number(x()));
    writer.writeAttribute("ypos", QString::number(y()));
    writer.writeAttribute("width", QString::number(rect().width()));
    writer.writeAttribute("height", QString::number(rect().height()));
    writer.writeAttribute("rotation", QString::number(rotation()));

    writer.writeStartElement("text");
    writer.writeAttribute("font", textItem->font().toString());
    writer.writeAttribute("align", QString::number(alignment));
    writer.writeAttribute("textColor", textItem->defaultTextColor().name());

    writer.writeCharacters(textItem->toPlainText());
    writer.writeEndElement(); // text

    writer.writeEndElement(); // textFrame
}
#endif
