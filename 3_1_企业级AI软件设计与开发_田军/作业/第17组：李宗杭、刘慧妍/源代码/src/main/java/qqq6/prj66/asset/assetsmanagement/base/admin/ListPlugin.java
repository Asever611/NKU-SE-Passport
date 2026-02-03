package qqq6.prj66.asset.assetsmanagement.base.admin;

import kd.bos.context.RequestContext;
import kd.bos.form.events.SetFilterEvent;
import kd.bos.list.plugin.AbstractListPlugin;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.user.UserServiceHelper;

public class ListPlugin extends AbstractListPlugin {

    @Override
    public void setFilter(SetFilterEvent e) {
        if (UserServiceHelper.getCurrentUserId() != 2300859876403838976L){
            e.addCustomQFilter(new QFilter(
                    "creator.id", QCP.equals, 230085987640383897L
            ));
        }

        super.setFilter(e);
    }
}
