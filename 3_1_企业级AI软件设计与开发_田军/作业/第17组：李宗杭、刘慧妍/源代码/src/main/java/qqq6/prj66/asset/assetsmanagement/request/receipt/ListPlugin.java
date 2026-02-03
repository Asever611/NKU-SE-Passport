package qqq6.prj66.asset.assetsmanagement.request.receipt;

import kd.bos.context.RequestContext;
import kd.bos.form.events.SetFilterEvent;
import kd.bos.list.plugin.AbstractListPlugin;
import kd.bos.orm.query.QCP;
import kd.bos.orm.query.QFilter;
import kd.bos.servicehelper.user.UserServiceHelper;

public class ListPlugin extends AbstractListPlugin {

    @Override
    public void setFilter(SetFilterEvent e) {
        super.setFilter(e);
        e.getQFilters();
        if (UserServiceHelper.getCurrentUserId() != 2300859876403838976L){
            long[] ids = new long[]{UserServiceHelper.getCurrentUserId()};
            e.addCustomQFilter(new QFilter(
                    "qqq6_requester.id", QCP.in, ids
            ));
        }


    }
}
