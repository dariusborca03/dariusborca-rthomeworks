package ssatr.device;
/** Darius Borca: snapshot la sosire, independent de modificarile ulterioare. */
public record AlarmEvent(double temperature, double threshold, long arrivalNanos) {}
