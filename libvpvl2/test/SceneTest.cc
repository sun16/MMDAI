#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu/Encoding.h"
#include "mock/Model.h"
#include "mock/Motion.h"
#include "mock/RenderEngine.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu;

TEST(SceneTest, AddModel)
{
    Scene scene;
    /* adding an null model should not be crashed */
    scene.addModel(0, 0);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    QScopedPointer<MockIModel> model(new MockIModel());
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    /* adding a model but no rendering engine should not be added */
    scene.addModel(model.data(), 0);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    /* no rendering context class will be referered */
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    /* adding a rendering engine but no model should not be added */
    scene.addModel(0, engine.data());
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
    scene.addModel(model.take(), engine.take());
    /* both model and rendering engine should be added */
    ASSERT_EQ(1, scene.models().count());
    ASSERT_EQ(1, scene.renderEngines().count());
}

TEST(SceneTest, AddMotion)
{
    /* no encoding class will be referered */
    Factory factory(0);
    Scene scene;
    /* adding an null motion should not be crashed */
    scene.addMotion(0);
    ASSERT_EQ(0, scene.motions().count());
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    scene.addMotion(motion.take());
    ASSERT_EQ(1, scene.motions().count());
}

TEST(SceneTest, FindModel)
{
    Scene scene;
    /* adding an null motion should not be crashed */
    scene.findModel(0);
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    QScopedPointer<MockIModel> model(new MockIModel());
    String s(UnicodeString::fromUTF8("foo_bar_baz")), s2(s.value());
    EXPECT_CALL(*model, name()).WillOnce(Return(&s));
    scene.addModel(model.data(), engine.take());
    ASSERT_EQ(model.take(), scene.findModel(&s2));
}

TEST(SceneTest, FindRenderEngine)
{
    Scene scene;
    QScopedPointer<MockIModel> model(new MockIModel());
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    scene.addModel(model.data(), engine.data());
    ASSERT_EQ(engine.take(), scene.findRenderEngine(model.take()));
}

TEST(SceneTest, DeleteModel)
{
    Scene scene;
    QScopedPointer<MockIModel> model(new MockIModel());
    String s(UnicodeString::fromUTF8("This is a test model."));
    EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
    QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
    IModel *fakePtr = 0;
    /* deleting an null model should not be crashed */
    scene.deleteModel(fakePtr);
    ASSERT_EQ(0, fakePtr);
    scene.addModel(model.data(), engine.take());
    IModel *modelPtr = model.data();
    /* model should be deleted and set it null */
    scene.deleteModel(modelPtr);
    model.take();
    ASSERT_EQ(0, modelPtr);
    ASSERT_EQ(0, scene.models().count());
    ASSERT_EQ(0, scene.renderEngines().count());
}

TEST(SceneTest, Update)
{
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateRenderEngines);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).WillOnce(Return());
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateAll);
    }
    {
        QScopedPointer<MockIRenderEngine> engine(new MockIRenderEngine());
        QScopedPointer<MockIModel> model(new MockIModel());
        EXPECT_CALL(*engine, update()).Times(0);
        String s(UnicodeString::fromUTF8("This is a test model."));
        EXPECT_CALL(*model, name()).WillRepeatedly(Return(&s));
        Scene scene;
        scene.addModel(model.take(), engine.take());
        scene.update(Scene::kUpdateCamera);
        scene.update(Scene::kUpdateLight);
        scene.update(Scene::kUpdateModels);
    }
}

TEST(SceneTest, RemoveMotion)
{
    Factory factory(0);
    Scene scene;
    /* removing an null motion should not be crashed */
    scene.removeMotion(0);
    ASSERT_EQ(0, scene.motions().count());
    QScopedPointer<IMotion> motion(factory.createMotion(IMotion::kVMD, 0));
    scene.addMotion(motion.data());
    /* motion should be removed and set it null */
    scene.removeMotion(motion.data());
    ASSERT_EQ(0, scene.motions().count());
}

TEST(SceneTest, AdvanceMotions)
{
    Scene scene;
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, advance(0)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, SeekMotions)
{
    Scene scene;
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateModels);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
        scene.removeMotion(&motion);
    }
    {
        MockIMotion motion; scene.addMotion(&motion);
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateRenderEngines);
        scene.removeMotion(&motion);
    }
}

TEST(SceneTest, Camera)
{
    QScopedPointer<ICamera> camera1(Scene::createCamera()), camera2(Scene::createCamera());
    Scene scene;
    ASSERT_NE(camera2.data(), camera1.data());
    ASSERT_EQ(scene.camera(), scene.camera());
}

TEST(SceneTest, AdvanceSceneCamera)
{
    Scene scene;
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateCamera);
    }
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
    }
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateLight);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
    }
    scene.camera()->setMotion(0);
}

TEST(SceneTest, SeekSceneCamera)
{
    Scene scene;
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateCamera);
    }
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
    }
    {
        MockIMotion motion; scene.camera()->setMotion(&motion);
        EXPECT_CALL(motion, seek(0)).Times(0);
        scene.seek(0, Scene::kUpdateLight);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
    }
    scene.camera()->setMotion(0);
}

TEST(SceneTest, Light)
{
    QScopedPointer<ILight> light1(Scene::createLight()), light2(Scene::createLight());
    Scene scene;
    ASSERT_NE(light2.data(), light1.data());
    ASSERT_EQ(scene.light(), scene.light());
}

TEST(SceneTest, AdvanceSceneLight)
{
    Scene scene;
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateLight);
    }
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).WillOnce(Return());
        scene.advance(0, Scene::kUpdateAll);
    }
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, advanceScene(0, &scene)).Times(0);
        scene.advance(0, Scene::kUpdateCamera);
        scene.advance(0, Scene::kUpdateModels);
        scene.advance(0, Scene::kUpdateRenderEngines);
    }
    scene.light()->setMotion(0);
}

TEST(SceneTest, SeekSceneLight)
{
    Scene scene;
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateLight);
    }
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).WillOnce(Return());
        scene.seek(0, Scene::kUpdateAll);
    }
    {
        MockIMotion motion; scene.light()->setMotion(&motion);
        EXPECT_CALL(motion, seekScene(0, &scene)).Times(0);
        scene.seek(0, Scene::kUpdateCamera);
        scene.seek(0, Scene::kUpdateModels);
        scene.seek(0, Scene::kUpdateRenderEngines);
    }
    scene.light()->setMotion(0);
}
