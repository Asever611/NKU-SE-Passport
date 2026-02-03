package qqq6.prj66.asset.assetsmanagement.summary.changed;

import kd.bos.entity.report.ReportQueryParam;
import kd.bos.report.filter.ReportFilter;
import kd.bos.report.plugin.AbstractReportFormPlugin;

public class FormPlugin extends AbstractReportFormPlugin {
    @Override
    public void afterQuery(ReportQueryParam queryParam) {
        ReportFilter reportfilterap = this.getView().getControl("reportfilterap");
        reportfilterap.setCollapse(false);
        super.afterQuery(queryParam);
    }
}
