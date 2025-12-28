/*
 * multipleofeightspinbox.h
 *
 * Author: Andrey Zhdanov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MULTIPLEOFEIGHTSPINBOX_H_
#define MULTIPLEOFEIGHTSPINBOX_H_

#include <QSpinBox>

class MultipleOfEightSpinBox : public QSpinBox {
    Q_OBJECT
    
public:
    MultipleOfEightSpinBox(QWidget *parent = nullptr) : QSpinBox(parent) {
        setSingleStep(8);
    }
    
    QString textFromValue(int value) const override {
        // Convert value to string
        return QString::number(value);
    }
    
    int valueFromText(const QString &text) const override {
        bool ok;
        int val = text.toInt(&ok);
        // Round to nearest multiple of 8 when user types
        if (ok) {
            return (val / 8) * 8;
        }
        return 0;
    }
};

#endif /* MULTIPLEOFEIGHTSPINBOX_H_ */
