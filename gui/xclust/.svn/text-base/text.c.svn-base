#include "xclust_ext.h"

Label *commandlabel=NULL;
Label *coordlabel = NULL;
Label *rotlabel = NULL;

RefreshText(text)
TextWindow	*text;
{
    ClearWindow(text);
    /*
    ** draw the 3d edges
    */
    _SetColor(text,MININTERFACECOLOR + MENUITEMDARKEDGE);
    _DrawLine(text,text->wwidth-2,text->wheight-2,
	text->wwidth-2,1);
    _DrawLine(text,text->wwidth-2,text->wheight-2,
	1,text->wheight-2);
    _SetColor(text,MININTERFACECOLOR + MENUITEMLIGHTEDGE);
    _DrawLine(text,1,1,1,text->wheight-2);
    _DrawLine(text,1,1,text->wwidth-2,1);

    DisplayLabels(text);
    /*
    ** draw the menu scroll buttons
    */
}

RepositionTextItems(text)
TextWindow	*text;
{
MenuItem	*item;
    item = LookupMenuItem("/text/scrollup");
    XMoveResizeWindow(item->display,item->window,
    item->menu->wwidth-20,0,20,20);
    item = LookupMenuItem("/text/scrolldown");
    XMoveResizeWindow(item->display,item->window,
    item->menu->wwidth-40,0,20,20);
}
