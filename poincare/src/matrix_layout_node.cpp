#include <poincare/matrix_layout_node.h>
#include <poincare/bracket_pair_layout_node.h>
#include <poincare/empty_layout_node.h>
#include <poincare/square_bracket_layout_node.h>
#include <poincare/layout_engine.h>

namespace Poincare {

// MatrixLayoutNode

void MatrixLayoutNode::newRowOrColumnAtIndex(int index) {
  assert(index >= 0 && index < m_numberOfColumns*m_numberOfRows);
  bool shouldAddNewRow = childIsBottomOfGrid(index); // We need to compute this boolean before modifying the layout
  int correspondingRow = rowAtChildIndex(index);
    if (childIsRightOfGrid(index)) {
    // Color the grey EmptyLayouts of the column in yellow.
    int correspondingColumn = m_numberOfColumns - 1;
    for (int i = 0; i < m_numberOfRows - 1; i++) {
      LayoutNode * lastLayoutOfRow = childAtIndex(i*m_numberOfColumns+correspondingColumn);
      if (lastLayoutOfRow->isEmpty()) {
        static_cast<EmptyLayoutNode *>(lastLayoutOfRow)->setColor(EmptyLayoutNode::Color::Yellow);
      }
    }
    // Add a column of grey EmptyLayouts on the right.
    addEmptyColumn(EmptyLayoutNode::Color::Grey);
  }
  if (shouldAddNewRow) {
    // Color the grey EmptyLayouts of the row in yellow.
    for (int i = 0; i < m_numberOfColumns - 1; i++) {
      LayoutNode * lastLayoutOfColumn = childAtIndex(correspondingRow*m_numberOfColumns+i);
      if (lastLayoutOfColumn->isEmpty()) {
        static_cast<EmptyLayoutNode *>(lastLayoutOfColumn)->setColor(EmptyLayoutNode::Color::Yellow);
      }
    }
    // Add a row of grey EmptyLayouts at the bottom.
    addEmptyRow(EmptyLayoutNode::Color::Grey);
  }
}

void MatrixLayoutNode::addGreySquares() {
  if (!hasGreySquares()) {
    addEmptyRow(EmptyLayoutNode::Color::Grey);
    addEmptyColumn(EmptyLayoutNode::Color::Grey);
  }
}

void MatrixLayoutNode::removeGreySquares() {
  if (hasGreySquares()) {
    deleteRowAtIndex(m_numberOfRows - 1);
    deleteColumnAtIndex(m_numberOfColumns - 1);
  }
}

// LayoutNode

void MatrixLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  int childIndex = indexOfChild(cursor->layoutNode());
  if (childIndex >= 0
      && cursor->position() == LayoutCursor::Position::Left
      && childIsLeftOfGrid(childIndex))
  {
    /* Case: Left of a child on the left of the grid. Remove the grey squares of
     * the grid, then go left of the grid. */
    assert(hasGreySquares());
    removeGreySquares();
    *shouldRecomputeLayout = true;
    cursor->setLayoutNode(this);
    return;
  }
  if (cursor->layoutNode() == this
      && cursor->position() == LayoutCursor::Position::Right)
  {
    /* Case: Right. Add the grey squares to the matrix, then move to the bottom
     * right non empty nor grey child. */
    assert(!hasGreySquares());
    addGreySquares();
    *shouldRecomputeLayout = true;
    LayoutNode * lastChild = childAtIndex((m_numberOfColumns-1)*(m_numberOfRows-1));
    assert(lastChild != nullptr);
    cursor->setLayoutNode(lastChild);
    return;
  }
  GridLayoutNode::moveCursorLeft(cursor, shouldRecomputeLayout);
}

void MatrixLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (cursor->layoutNode() == this
      && cursor->position() == LayoutCursor::Position::Left)
  {
    // Case: Left. Add grey squares to the matrix, then go to its first entry.
    assert(!hasGreySquares());
    addGreySquares();
    *shouldRecomputeLayout = true;
    assert(m_numberOfColumns*m_numberOfRows >= 1);
    assert(childAtIndex(0) != nullptr);
    cursor->setLayoutNode(childAtIndex(0));
    return;
  }
  int childIndex = indexOfChild(cursor->layoutNode());
  if (childIndex >= 0
      && cursor->position() == LayoutCursor::Position::Right
      && childIsRightOfGrid(childIndex))
  {
    /* Case: Right of a child on the right of the grid. Remove the grey squares
     * of the grid, then go right of the grid. */
    assert(hasGreySquares());
    removeGreySquares();
    *shouldRecomputeLayout = true;
    cursor->setLayoutNode(this);
    return;

  }
  GridLayoutNode::moveCursorRight(cursor, shouldRecomputeLayout);
}


// SerializableNode

int MatrixLayoutNode::writeTextInBuffer(char * buffer, int bufferSize, PrintFloat::Mode floatDisplayMode, int numberOfSignificantDigits) const {
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;
  if (bufferSize == 1) {
    return 1;
  }
  int numberOfChar = 0;
  buffer[numberOfChar++] = '[';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1;}

  int maxRowIndex = hasGreySquares() ? m_numberOfRows - 1 : m_numberOfRows;
  int maxColumnIndex = hasGreySquares() ? m_numberOfColumns - 2 :  m_numberOfColumns - 1;
  for (int i = 0; i < maxRowIndex; i++) {
    buffer[numberOfChar++] = '[';
    if (numberOfChar >= bufferSize-1) { return bufferSize-1;}

    numberOfChar += LayoutEngine::writeInfixSerializableRefTextInBuffer(SerializableRef(const_cast<MatrixLayoutNode *>(this)), buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits, ",", i*m_numberOfColumns, i* m_numberOfColumns + maxColumnIndex);
    if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

    buffer[numberOfChar++] = ']';
    if (numberOfChar >= bufferSize-1) { return bufferSize-1; }
  }
  buffer[numberOfChar++] = ']';
  buffer[numberOfChar] = 0;
  return numberOfChar;
}

// Protected

void MatrixLayoutNode::computeSize() {
  assert(!m_sized);
  KDSize sizeWithoutBrackets = gridSize();
  KDSize sizeWithBrackets = KDSize(
      sizeWithoutBrackets.width() + 2 * SquareBracketLayoutNode::BracketWidth(),
      sizeWithoutBrackets.height() + 2 * SquareBracketLayoutNode::k_lineThickness);
  m_frame.setSize(sizeWithBrackets);
  m_sized = true;
}

KDPoint MatrixLayoutNode::positionOfChild(LayoutNode * l) {
  assert(indexOfChild(l) >= 0);
  LayoutRef dummyGridLayout = GridLayoutRef(this).clone();
  BracketPairLayoutRef dummyLayout(dummyGridLayout);
  return GridLayoutNode::positionOfChild(l).translatedBy(dummyLayout.positionOfChild(dummyGridLayout));
}

void MatrixLayoutNode::moveCursorVertically(VerticalDirection direction, LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited) {
  bool shouldRemoveGreySquares = false;
  int firstIndex = direction == VerticalDirection::Up ? 0 : numberOfChildren() - m_numberOfColumns;
  int lastIndex = direction == VerticalDirection::Up ? m_numberOfColumns : numberOfChildren();
  for (int childIndex = firstIndex; childIndex < lastIndex; childIndex++) {
    if (cursor->layoutNode()->hasAncestor(childAtIndex(childIndex), true)) {
      // The cursor is leaving the matrix, so remove the grey squares.
      shouldRemoveGreySquares = true;
      break;
    }
  }
   GridLayoutNode::moveCursorVertically(direction, cursor, shouldRecomputeLayout, equivalentPositionVisited);
  if (cursor->isDefined() && shouldRemoveGreySquares) {
    assert(hasGreySquares());
    removeGreySquares();
    *shouldRecomputeLayout = true;
  }
}

// Private

bool MatrixLayoutNode::isRowEmpty(int index) const {
  assert(index >= 0 && index < m_numberOfRows);
  for (int i = index * m_numberOfColumns; i < (index+1) * m_numberOfColumns; i++) {
    if (!const_cast<MatrixLayoutNode *>(this)->childAtIndex(i)->isEmpty()) {
      return false;
    }
  }
  return true;
}

bool MatrixLayoutNode::isColumnEmpty(int index) const {
  assert(index >= 0 && index < m_numberOfColumns);
  for (int i = index; i < m_numberOfRows * m_numberOfColumns; i+= m_numberOfColumns) {
    if (!const_cast<MatrixLayoutNode *>(this)->childAtIndex(i)->isEmpty()) {
      return false;
    }
  }
  return true;
}

bool MatrixLayoutNode::hasGreySquares() const {
  assert(m_numberOfRows*m_numberOfColumns - 1 >= 0);
  LayoutNode * lastChild = const_cast<MatrixLayoutNode *>(this)->childAtIndex(m_numberOfRows * m_numberOfColumns - 1);
  if (lastChild->isEmpty()
      && !lastChild->isHorizontal()
      && (static_cast<EmptyLayoutNode *>(lastChild))->color() == EmptyLayoutNode::Color::Grey)
  {
    assert(isRowEmpty(m_numberOfRows - 1));
    assert(isColumnEmpty(m_numberOfColumns - 1));
    return true;
  }
  return false;
}

void MatrixLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) {
  BracketPairLayoutNode::RenderWithChildSize(gridSize(), ctx, p, expressionColor, backgroundColor);
  GridLayoutNode::render(ctx, p.translatedBy(KDPoint(SquareBracketLayoutNode::BracketWidth(), SquareBracketLayoutNode::k_lineThickness)), expressionColor, backgroundColor);
}

void MatrixLayoutNode::didReplaceChildAtIndex(int index, LayoutCursor * cursor, bool force) {
  assert(index >= 0 && index < m_numberOfColumns*m_numberOfRows);
  if (cursor) {
    cursor->setLayoutNode(childAtIndex(index));
    cursor->setPosition(LayoutCursor::Position::Right);
  }
  int rowIndex = rowAtChildIndex(index);
  int columnIndex = columnAtChildIndex(index);
  bool rowIsEmpty = isRowEmpty(rowIndex);
  bool columnIsEmpty = isColumnEmpty(columnIndex);
  if (rowIsEmpty && m_numberOfRows > 2) {
    deleteRowAtIndex(rowIndex);
  }
  if (columnIsEmpty && m_numberOfColumns > 2) {
    deleteColumnAtIndex(columnIndex);
  }
  if (!rowIsEmpty && !columnIsEmpty) {
    LayoutNode * newChild = childAtIndex(index);
    if (newChild->isEmpty()
        && (childIsRightOfGrid(index)|| childIsBottomOfGrid(index)))
    {
      static_cast<EmptyLayoutNode *>(newChild)->setColor(EmptyLayoutNode::Color::Grey);
    }
  }
}

}