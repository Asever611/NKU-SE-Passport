package qqq6.prj66.asset.assetsmanagement.home.agentcard;

import kd.bos.form.control.Control;
import kd.bos.form.control.Image;
import kd.bos.form.plugin.AbstractFormPlugin;

import java.net.URL;
import java.util.EventObject;

public class MainPlugin extends AbstractFormPlugin {

    private static final String KEY_IMAGE = "qqq6_image";
    private static final String IMAGEURL = "pictures/助手头像.png";
    private static final String AGENTURL = "http://127.0.0.1:8080/ierp/ai/h5/chat.do?accountId=1565321489509515264&assistant=2319965809889250304";

//    @Override
//    public void afterBindData(EventObject e) {
//        super.afterBindData(e);
//        Image image = this.getView().getControl(KEY_IMAGE);
//        URL imageURL = getClass().getClassLoader().getResource(IMAGEURL);
//        String URLString = imageURL.toString();
//        image.setUrl(URLString);
//    }

    @Override
    public void registerListener(EventObject e) {
        super.registerListener(e);
        Image image = this.getView().getControl(KEY_IMAGE);
        image.addClickListener(this);
    }

    @Override
    public void click(EventObject evt) {
        super.click(evt);
        Control source = (Control) evt.getSource();
        if (KEY_IMAGE.equals(source.getKey())) {
            this.getView().openUrl(AGENTURL);
        }
    }
}