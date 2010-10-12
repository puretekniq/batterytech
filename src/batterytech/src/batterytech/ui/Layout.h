/*
 * Layout.h
 *
 *  Created on: Sep 24, 2010
 *      Author: rgreen
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include "UIComponent.h"

class Layout: public UIComponent {
public:
	Layout(const char *text = NULL) : UIComponent(text){};
	virtual ~Layout();
};

#endif /* LAYOUT_H_ */
