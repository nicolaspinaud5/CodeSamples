package com.example.java2com;

import java.util.Iterator;


public abstract class COMCollection<T> extends IDispatch implements Iterable<T> {

    protected COMCollection(COMProxy ci) {
        super(ci);
    }

    
    protected abstract T newInstance(COMProxy dispatch); 

    public int getCount() {
        Variant result = ci.getProperty("Count");
        return result.intValue;
    }


    public Iterator<T> iterator() {
        return new Iterator<T>() {

            int idx = 1;
            int count = getCount();

            @Override
            public boolean hasNext() {
                return idx <= count;
            }

            @Override
            public T next() {
                return getItem(idx++);
            }

        };
    }
    

    public T getItem(String id) {
        Variant result = ci.getProperty("Item", new Variant(id));
        return newInstance(result.dispatch);
    }

    
    public T getItem(int idx) {
        Variant result = ci.getProperty("Item", new Variant(idx));
        return newInstance(result.dispatch);
    }
}
