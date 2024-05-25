package test.gyakorlat2024.lab07;
import static check.CheckThat.*;
import static check.CheckThat.Condition.*;
import org.junit.jupiter.api.*;
import check.CheckThat;

public class MultiLineFileStructureTest {
    @BeforeAll
    public static void init() {
        CheckThat.theClass("text.to.numbers.MultiLineFile")
            .thatIs(FULLY_IMPLEMENTED, INSTANCE_LEVEL, VISIBLE_TO_ALL);
    }

    @Test
    public void constructor() {
        it.hasNoArgConstructor()
            .thatIs(VISIBLE_TO_ALL);
    }

    @Test
    public void methodAddNumbers() {
        it.hasMethodWithParams("addNumbers", "String", "char")
            .thatIs(FULLY_IMPLEMENTED, USABLE_WITHOUT_INSTANCE, VISIBLE_TO_ALL)
            .thatReturns("int");
    }
}

